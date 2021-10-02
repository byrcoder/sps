/*****************************************************************************
MIT License
Copyright (c) 2021 byrcoder

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*****************************************************************************/

//
// Created by byrcoder on 2021/10/2.
//

#include <sps_avformat_rtpdec_rtcp.hpp>
#include <sps_log.hpp>

namespace sps {

error_t RtcpHeadDecoder::decode(BitContext &bc) {
    error_t ret = bc.acquire(4);

    if (ret != SUCCESS) {
        sp_error("[rtp@rtcp] header size < 4");
        return ERROR_RTP_PROBE;
    }

    header.version = bc.read_bits(2);
    header.padding = bc.read_bits(1);
    header.cflag   = bc.read_bits(5);
    header.payload_type = bc.read_int8();
    header.length       = bc.read_int16();

    return ret;
}

std::string IRtcpDecoder::debug() {
    return "";
}

error_t RtcpSRReportBlock::decode(BitContext &bc) {
    error_t ret = bc.acquire(20);

    if (ret != SUCCESS) {
        return ret;
    }

    fraction_lost = bc.read_int8();
    cum_number_pkt_lost = bc.read_int24();
    extend_high_seq_num_recv = bc.read_int32();
    inter_arrival_jitter = bc.read_int32();
    last_sr = bc.read_int32();
    delay_since_last_sr = bc.read_int32();
    return ret;
}

error_t RtcpSRDecoder::decode(RtcpHead& h, BitContext &bc) {
    //   head + number * sizeof(RtcpSRReportBlock)
    int size    = 24 + h.cflag * RtcpSRReportBlock::SIZE;
    error_t ret = SUCCESS;

    if ((ret = bc.acquire(size)) != SUCCESS) {
        sp_error("[rtp@rtcp] SR report block %d, ret %d", h.cflag, ret);
        return ret;
    }

    ssrc = bc.read_int32();
    ntp_ts_msw = bc.read_int32();
    ntp_ts_lsw = bc.read_int32();
    rtp_ts     = bc.read_int32();
    send_pkt_count = bc.read_int32();
    send_oct_count = bc.read_int32();

    for (int i = 0; i < h.cflag; ++i) {
        RtcpSRReportBlock rb;
        rb.decode(bc);
        report_blocks.push_back(rb);
    }

    return ret;
}

std::string RtcpSRDecoder::debug() {
    thread_local char buf[128];
    int n = snprintf(buf, sizeof(buf), " ssrc %x, msw %u, lsw %u, rtp_ts %u, pkt %u, oc %u",
            ssrc, ntp_ts_msw, ntp_ts_lsw, rtp_ts, send_pkt_count, send_oct_count);
    return std::string(buf, n);
}

error_t RtcpRRDecoder::decode(RtcpHead &h, BitContext &bc) {
    int size    = 4 + h.cflag * RtcpSRReportBlock::SIZE;;
    error_t ret = SUCCESS;

    if ((ret = bc.acquire(size)) != SUCCESS) {
        sp_error("[rtp@rtcp] SR report block %d, ret %d", h.cflag, ret);
        return ret;
    }

    ssrc = bc.read_int32();

    for (int i = 0; i < h.cflag; ++i) {
        RtcpSRReportBlock rb;
        rb.decode(bc);
        report_blocks.push_back(rb);
    }

    return ret;
}

std::string RtcpRRDecoder::debug() {
    thread_local char buf[128];
    int n = snprintf(buf, sizeof(buf), " ssrc %x",
                     ssrc);
    return std::string(buf, n);
}

error_t RtcpSDES::decode(BitContext &bc) {
    error_t ret = bc.acquire(6);

    if (ret != SUCCESS) {
        return ret;
    }

    ssrc = bc.read_int32();
    item_type = bc.read_int8();
    item_length = bc.read_int8();

    if ((ret = bc.acquire(item_length)) != SUCCESS) {
        return ret;
    }

    item_value = CharBuffer::create_buf(item_length);
    item_value->append(bc.pos(), item_length);
    bc.skip_read_bytes(item_length);

    return ret;
}

error_t RtcpSDESDecoder::decode(RtcpHead &h, BitContext &bc) {
    static RtcpSDES sd;
    error_t ret = SUCCESS;

    for (int i = 0; i < h.cflag; ++i) {
        sdes.push_back(sd);
        auto it = sdes.end();

        if ((ret = it->decode(bc)) != SUCCESS) {
            sp_error("[rtp@rtcp] sdes decode %dth fail num %d, ret %d", i, (int) h.cflag, ret);
            return ret;
        }
    }

    return ret;
}

std::string RtcpSDESDecoder::debug() {
    thread_local char buf[128];
    int n = snprintf(buf, sizeof(buf), " ssrc %lu",
                     sdes.size());
    return std::string(buf, n);
}

error_t RtcpByteDecoder::decode(RtcpHead &h, BitContext &bc) {
    // ssrc + length(1byte) + reason for live
    bc.skip_read_bytes(h.length);
    return SUCCESS;
}

std::string RtcpByteDecoder::debug() {
    return "";
}

error_t RtcpAppDecoder::decode(RtcpHead &h, BitContext &bc) {
    // ssrc + name(4byte) + application
    bc.skip_read_bytes(h.length);
    return SUCCESS;
}

std::string RtcpAppDecoder::debug() {
    return "";
}

error_t RtcpRtpDecoder::decode(uint8_t *buffer, int size, std::list<PAVPacket> &pkts) {
    RtcpHeadDecoder dc;
    BitContext bc(buffer, size);
    error_t ret = dc.decode(bc);

    if (ret != SUCCESS) {
        return ret;
    }

    PIRtcpDecoder rtcp_dc;
    switch (dc.header.payload_type) {
        case RTP_SR:
            rtcp_dc = std::make_shared<RtcpSRDecoder>();
            break;
        case RTP_RR:
            rtcp_dc = std::make_shared<RtcpRRDecoder>();
            break;
        case RTP_SDES:
            rtcp_dc = std::make_shared<RtcpSDESDecoder>();
            break;
        case RTP_BYE:
            rtcp_dc = std::make_shared<RtcpByteDecoder>();
            break;
        case RTP_APP:
            rtcp_dc = std::make_shared<RtcpAppDecoder>();
            break;
        default:
            sp_error("fatal unknow rtcp payload type %d", dc.header.payload_type);
            return ERROR_RTP_PROBE;
    }

    if ((ret = rtcp_dc->decode(dc.header, bc)) != SUCCESS) {
        return ret;
    }

    sp_trace("[rtp@rtcp] pt %d, %s", dc.header.payload_type, rtcp_dc->debug().c_str());
    return ret;
}

bool RtcpRtpDecoder::match(int pt) {
    return is_rtcp(pt);
}

error_t RtcpRtpDecoder::decode(RtpPayloadHeader &header, BitContext &bc, std::list<PAVPacket> &pkts) {
    return SUCCESS;
}

}
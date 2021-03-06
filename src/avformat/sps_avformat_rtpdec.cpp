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
// Created by byrcoder on 2021/9/25.
//

#include <sps_avformat_rtpdec.hpp>

#include <sps_avcodec_parser.hpp>
#include <sps_avformat_rtp.hpp>
#include <sps_avformat_rtpdec_aac.hpp>
#include <sps_avformat_rtpdec_h264_hevc.hpp>
#include <sps_avformat_rtpdec_rtcp.hpp>
#include <sps_avformat_rtpdec_ts.hpp>
#include <sps_log.hpp>

namespace sps {

error_t IRtpDecoder::decode(uint8_t *buffer, int size, std::list<PAVPacket> &pkts) {
    BitContext       bc(buffer, size);
    RtpPayloadHeader rtp_header;
    error_t          ret = SUCCESS;

    if ((ret = bc.acquire(12)) != SUCCESS) {
        return ret;
    }

    if ((ret = rtp_header.decode(bc)) != SUCCESS) {
        return ret;
    }

    if (!match(rtp_header.fix_header.payload_type)) {
        sp_error("payload not match %d", rtp_header.fix_header.payload_type);
        return ERROR_RTP_PAYLOAD_TYPE;
    }

    return decode(rtp_header, bc, pkts);
}

bool IRtpDynamicDecoder::match(int pt) {
    return is_rtp_dynamic(pt);
}

RtpDemuxer::RtpDemuxer(PIReader reader) {
    buf       = std::make_shared<AVBuffer>(RTP_MAX_LENGTH, true);
    rd        = std::make_unique<BytesReader>(reader, buf);
    rtcp_decoder = std::make_shared<RtcpRtpDecoder>();
}

error_t RtpDemuxer::init_codec(int codec_id) {
    error_t ret = SUCCESS;

    switch (codec_id) {
        case AVCODEC_H264:
            rtp_decoder = std::make_shared<H264RtpDecoder>();
            break;
        case AVCODEC_AAC:
            rtp_decoder = std::make_shared<AacRtpDecoder>();
        default:
            sp_error("not found codec %d", codec_id);
            return ERROR_RTP_PROBE;
    }

    return ret;
}

error_t RtpDemuxer::init_rtp_decode(int payload_type) {
    rtp_decoder.reset();

    if (payload_type == RTP_TS) {
        rtp_decoder = std::make_shared<MpegtsRtpDecodder>();
    }

    return rtp_decoder ? SUCCESS : ERROR_RTP_NOT_SUPPORT;
}

error_t RtpDemuxer::read_header(PAVPacket & buffer) {
    return SUCCESS;
}

error_t RtpDemuxer::read_packet(PAVPacket& buffer) {
    error_t ret = SUCCESS;

    do {
        buf->clear();  // clear pkt pos

        if (!pkts.empty()) {
            buffer = pkts.front();
            pkts.erase(pkts.begin());
            return ret;
        }

        RtpPayloadHeader rtp_header;
        if ((ret = rd->acquire(12)) != SUCCESS) {
            return ret;
        }

        uint8_t* p   = rd->pos();
        size_t   len = rd->size();

        uint8_t pay_type = p[1];
        if (is_rtcp(pay_type)) {
            if ((ret = rtcp_decoder->decode(p, len, pkts)) != SUCCESS) {
                return ret;
            }
            continue;
        }

        if (!rtp_decoder && (ret = init_rtp_decode(pay_type)) != SUCCESS) {
            sp_error("[rtp] decoder not init ret %d", ret);
            return ERROR_RTP_PROBE;
        }

        if ((ret = rtp_decoder->decode(p, len, pkts)) != SUCCESS) {
            return ret;
        }

    } while (true);

    return ret;
}

error_t RtpDemuxer::read_tail(PAVPacket& buffer) {
    return SUCCESS;
}

error_t RtpDemuxer::probe(PAVPacket& buffer) {
    return ERROR_RTP_PROBE;
}

}
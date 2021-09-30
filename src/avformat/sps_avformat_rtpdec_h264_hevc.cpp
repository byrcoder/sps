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
// Created by byrcoder on 2021/9/26.
//

#include <sps_avformat_rtpdec_h264_hevc.hpp>
#include <sps_log.hpp>
#include <sps_util_time.hpp>

namespace sps {

FuDecoder::FuDecoder() {
}

error_t FuDecoder::decode(BitContext &bc) {
    error_t ret = SUCCESS;

    if ((ret = bc.acquire(2)) != SUCCESS) {
        sp_error("[rtp@h264] fu-decode size empty ret %d", ret);
        return ret;
    }

    indicator.f = bc.read_bits(1);
    indicator.nri = bc.read_bits(2);
    indicator.tp = bc.read_bits(5);

    header.s = bc.read_bits(1);
    header.e = bc.read_bits(1);
    header.r = bc.read_bits(1);
    header.tp = bc.read_bits(5);

    return ret;
}

bool FuDecoder::is_start() {
    return header.s == 1;
}

bool FuDecoder::is_end() {
    return header.e == 1;
}

error_t NaluRtpDecoder::decode(int tp, RtpPayloadHeader &header,
                               BitContext &bc, std::list<PAVPacket> &pkts) {
    // TODO: fixme dts, use clock timestamp
    AVCodecContext ctx(get_time() * 90, header.fix_header.timestamp, 90);
    H264AVCodecParser h264_codec;
    auto ret = h264_codec.encode_raw_nalu(&ctx, bc.pos(), bc.size(), pkts);

    if (ret != SUCCESS) {
        sp_error("[rtp@h264] nalu-decode ret %d", ret);
        return ret;
    }

    bc.skip_read_bytes(bc.size());
    return ret;
}

error_t StapRtpDecoder::decode(int tp, RtpPayloadHeader &header,
                               BitContext &bc, std::list<PAVPacket> &pkts) {
    error_t ret = SUCCESS;

    if (tp == STAP_B) {
        bc.skip_read_bytes(2);
    }

    while (bc.size() > 0) {
        int size = bc.read_int8();

        if ((ret = bc.acquire(size)) != SUCCESS) {
            return ret;
        }

        NaluRtpDecoder dc;
        ret = dc.decode(tp, header, bc, pkts);
        if (ret != SUCCESS) {
            sp_error("[rtp@h264] stap-decode ret %d", ret);
            return ret;
        }
    }

    return ret;
}

// TODO: fixme dts, pts timestamp
error_t MtapRtpDecoder::decode(int tp, RtpPayloadHeader &header,
                               BitContext &bc, std::list<PAVPacket> &pkts) {
    error_t ret = SUCCESS;
    int offset_len = tp == MTAP16 ? 2 : 3;

    if (tp == MTAP24) {
        bc.skip_read_bytes(3); // donb decoding order number base
    }

    while (bc.size() > 0) {
        //  mtap header
        //  nalu_size(1) + nalu_dond(1) +  nalu_ts_offset(16 or 24)
        if ((ret = bc.acquire(1 + 1 + offset_len)) != SUCCESS) {
            return ret;
        }

        auto nalu_size = bc.read_int8();
        auto nalu_dond = bc.read_int8();
        auto nalu_ts_offset = offset_len == 3 ? bc.read_int24() : bc.read_int16();

        if ((ret = bc.acquire(nalu_size)) != SUCCESS) {
            return ret;
        }

        NaluRtpDecoder dc;
        ret = dc.decode(tp, header, bc, pkts);
        if (ret != SUCCESS) {
            sp_error("[rtp@h264] mtap-decode ret %d", ret);
            return ret;
        }
    }
    return ret;
}

FuRtpDecoder::FuRtpDecoder(PCharBuffer &pkt, int64_t ts) : pkt(pkt) {
    this->pre_rtp_timestamp = ts;
}

error_t FuRtpDecoder::decode(int tp, RtpPayloadHeader &header,
        BitContext &bc, std::list<PAVPacket> &pkts) {
    error_t ret = SUCCESS;
    FuDecoder fu;
    if ((ret = fu.decode(bc)) != SUCCESS) {
        sp_error("[rtp@h264] fu-decode ret %d", ret);
        return ret;
    }

    if (fu.indicator.tp == FU_B) {
        bc.skip_read_bytes(2); // don
    }
    // start nalu
    if (fu.is_start()) {
        sp_debug("fu starting tp %d, indicator %d", tp, fu.indicator.tp);

        if (pkt) {
            return ERROR_RTP_DECODER_H264;
            //            NaluRtpDecoder dc;
            //            RtpPayloadHeader tmp_header;
            //            tmp_header.fix_header.timestamp = pre_rtp_timestamp;
            //            BitContext nbc(pkt->pos(), pkt->size());
            //            ret = dc.decode(tp, tmp_header, nbc, pkts);
            //            if (ret != SUCCESS) {
            //                sp_error("[rtp@h264] fu-decode ret %d", ret);
            //                return ret;
            //            }
        }
        pkt = std::make_shared<CharBuffer>(RTP_NALU_MAX_LENGTH, 0);

        uint8_t nal_header = (int32_t) (fu.indicator.f << 7) | (fu.indicator.nri << 5) | fu.header.tp;

        sp_info("fu start nal header %x", nal_header);
        // nalu header append, raw nalu
        pkt->append(&nal_header, 1);
    }

    // ingore some pkt because of udp
    if (!pkt) {
        sp_warn("[rtp@h264] fu-decode pkt is null ret %d", ret);
        return SUCCESS;
    }

    // nalu payload
    pkt->append(bc.pos(), bc.size());
    bc.skip_read_bytes(bc.size());

    // end nalu
    if (fu.is_end()) {
        sp_debug("fu ending");

        NaluRtpDecoder dc;
        BitContext nbc(pkt->buffer(), pkt->size());
        ret = dc.decode(tp, header, nbc, pkts);

        if (ret != SUCCESS) {
            sp_error("[rtp@h264] fu-decode ret %d", ret);
            return ret;
        }

        // free the pkt
        pkt.reset();
    }

    return ret;
}

PIRtpDecoder H264RtpDecoder::create_decoder(BitContext& bc, uint8_t nal_unit_type) {
    /**
     * Single NAL Unit Packet: Contains only a single NAL unit in the
     * payload.  The NAL header type field is equal to the original NAL unit
     * type, i.e., in the range of 1 to 23, inclusive.  Specified in Section
     * 5.6.
     */
    if (ZERO_RESERVED < nal_unit_type
        && nal_unit_type < STAP_A) {
        return std::make_shared<NaluRtpDecoder>();
    }

    switch (nal_unit_type) {
        case STAP_B:
        case STAP_A:
            return  std::make_shared<StapRtpDecoder>();
            break;
        case MTAP16:
        case MTAP24:
            return  std::make_shared<MtapRtpDecoder>();
            break;
        case FU_A:
        case FU_B:
            return  std::make_shared<FuRtpDecoder>(pkt, rtp_timestamp);
            break;
        default:
            sp_error("fail ingore nalu type %d", nal_unit_type);
            return nullptr;
    }
}

/**
 *     +---------------+
 *     |0|1|2|3|4|5|6|7|
 *     +-+-+-+-+-+-+-+-+
 *     |F|NRI|  Type   |
 *     +---------------+
 *     @param bc
 *     @param pkts
 *     @return
 */
error_t H264RtpDecoder::decode(RtpPayloadHeader& header, BitContext& bc, std::list<PAVPacket> &pkts) {
    error_t ret = SUCCESS;

    H264NAL nal;
    if ((ret = bc.acquire(1)) != SUCCESS) {
        sp_error("[rtp@h264] nalu parse fail ret %d", ret);
        return ret;
    }

    uint8_t nalu_type = bc.pos()[0] & 0x1f;
    auto dc = create_decoder(bc, nalu_type);

    if (!dc) {
        sp_error("[rtp@h264] null decode for nalu %d", nalu_type);
        return ERROR_RTP_PROBE;
    }

    if ((ret = dc->decode(nalu_type, header, bc, pkts)) != SUCCESS) {
        sp_error("[rtp@h264] decode nalu type %d fail ret %d", nalu_type, ret);
        return ret;
    }

    this->rtp_timestamp = header.fix_header.timestamp;

    return ret;
}

}
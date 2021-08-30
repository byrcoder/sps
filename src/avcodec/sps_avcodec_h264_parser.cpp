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
// Created by byrcoder on 2021/7/30.
//

#include <sps_avcodec_h264_parser.hpp>

#include <sps_log.hpp>

#include <sps_io_bits.hpp>

namespace sps {

bool H264NAL::is_pps() const {
    return H264_NAL_PPS == nal_header.nal_unit_type;
}

bool H264NAL::is_sps() const {
    return H264_NAL_SPS == nal_header.nal_unit_type;
}

bool H264NAL::is_idr() const {
    return H264_NAL_IDR_SLICE == nal_header.nal_unit_type;
}

bool H264NAL::is_aud() const {
    return H264_NAL_AUD == nal_header.nal_unit_type;
}

bool H264NAL::is_sei() const {
    return H264_NAL_SEI == nal_header.nal_unit_type;
}

bool H264NAL::is_slice() const {
    return H264_NAL_SLICE == nal_header.nal_unit_type;
}

error_t H264NAL::parse_nal(uint8_t *buf, size_t sz) {
    this->nalu_buf  = buf;
    this->nalu_size = sz;

    if (sz < 1) {
        sp_error("nalu size empty");
        return ERROR_H264_DECODER;
    }

    BitContext bc(buf, 1);
    nal_header.forbidden_bit     = bc.read_bits(1);
    nal_header.nal_reference_bit = bc.read_bits(2);
    nal_header.nal_unit_type     = bc.read_bits(5);

    if (nal_header.forbidden_bit != 0) {
        sp_error("invalid forbidden bit");
        return ERROR_H264_DECODER;
    }

    sp_debug("nalu type %x, sz %lu", nal_header.nal_unit_type, sz);
    return SUCCESS;
}

NALUContext::NALUContext(int64_t dts, int64_t pts, int64_t timebase) {
    this->dts = dts;
    this->pts = pts;
    this->timebase = timebase;
}

error_t NALUContext::append(uint8_t* buf, size_t sz) {
    H264NAL nal;
    error_t ret = nal.parse_nal(buf, sz);
    nalu_list.push_back(nal);
    return ret;
}

NALUParser::NALUParser(bool avc, int nsz) {
    is_avc                     = avc;
    nalu_length_size_minus_one = nsz;
}

uint8_t* find_start_code(uint8_t* buf, size_t sz) {
    while (sz > 3) {
        if (buf[0] == 0 && buf[1] == 0 && buf[2] == 1) {
            return buf;
        }
        buf += 1;
        sz -= 1;
    }
    return nullptr;
}

error_t NALUParser::decode(uint8_t* buf, size_t sz,
                           NALUContext* ctx,
                           bool is_seq_header) {
    if (is_seq_header) {
        return decode_header(buf, sz, ctx);
    }

    return decode_data(buf, sz, ctx);
}

error_t NALUParser::decode_header(uint8_t* buf, size_t sz,
                                  NALUContext* ctx) {
    if (!buf || sz < 1) {
        sp_error("fatal empty size");
        return ERROR_H264_DECODER;
    }

    BitContext bc(buf, sz);
    error_t    ret = SUCCESS;

    if (buf[0] == 1) {
        if (bc.acquire(7)) {
            sp_error("avc size < 7");
            return ERROR_H264_DECODER;
        }

        is_avc = bc.read_int8();  // version

        bc.read_int8();  // avc profile
        bc.read_int8();  // avc compatibility
        bc.read_int8();  // avc level
        bc.read_bits(6);  //  reserved
        nalu_length_size_minus_one = bc.read_bits(2);
        bc.read_bits(3);

        int sps_num = bc.read_bits(5);
        for (int i = 0; i < sps_num; ++i) {
            int nalu_size = bc.read_int16() + 2;
            if ((ret = bc.acquire(nalu_size)) != SUCCESS) {
                sp_error("fail acquire nalu size %d, ret %d", nalu_size, ret);
                return ERROR_H264_DECODER;
            }

            if ((ret = decode_data(bc.pos(), nalu_size, ctx)) != SUCCESS) {
                return ret;
            }
            bc.skip_read_bytes(nalu_size);
        }

        int pps_num = bc.read_int8();
        for (int i = 0; i < pps_num; ++i) {
            int nalu_size = bc.read_int16() + 2;
            if ((ret = bc.acquire(nalu_size)) != SUCCESS) {
                sp_error("fail acquire nalu size %d, ret %d", nalu_size, ret);
                return ERROR_H264_DECODER;
            }

            if ((ret = decode_data(bc.pos(), nalu_size, ctx)) != SUCCESS) {
                return ret;
            }
            bc.skip_read_bytes(nalu_size);
        }
    } else {
        is_avc = false;
        return decode_data(buf, sz, ctx);
    }

    return ret;
}

error_t NALUParser::decode_data(uint8_t* buf, size_t sz,
                                NALUContext* ctx) {
    error_t ret = SUCCESS;
    if (is_avc && nalu_length_size_minus_one < 0) {
        sp_error("nalu length size %d < 1", nalu_length_size_minus_one);
        return ERROR_H264_DECODER;
    }

    BitContext bc(buf, sz);
    uint8_t* tail = bc.pos() + bc.size();

    while (bc.acquire(4) == SUCCESS) {
        size_t nalu_size = 0;

        if (is_avc) {
            nalu_size = bc.read_bits((nalu_length_size_minus_one + 1) * 8);
        } else {
            uint8_t* start_pos = find_start_code(bc.pos(), bc.size());
            if (!start_pos) {
                sp_error("not found start code 00 00 01");
                return ERROR_H264_DECODER;
            }
            uint8_t* end_pos = find_start_code(start_pos + 1, bc.pos() + bc.size() - start_pos - 1);
            nalu_size = end_pos ? end_pos - start_pos - 3 : (tail - start_pos - 3);

            if (!end_pos) {
                sp_debug("not found next nalu, push full");
            }
            bc.skip_read_bytes(start_pos - bc.pos() + 3);
        }

        if (bc.acquire(nalu_size) != SUCCESS) {
            sp_error("fail acquire nalu size %lu", nalu_size);
            return ERROR_H264_DECODER;
        }

        if ((ret = ctx->append(bc.pos(), nalu_size)) != SUCCESS) {
            return ret;
        }
        bc.skip_read_bytes(nalu_size);
    }

    return SUCCESS;
}

error_t NALUParser::encode_avc(NALUContext *ctx, std::list<PAVPacket>& pkts) {
    H264NAL* sps    = nullptr;
    H264NAL* pps    = nullptr;

    for (auto& n : ctx->nalu_list) {
        if (n.is_sps()) {
            sps = &n;
        } else if (n.is_pps()) {
            pps = &n;
        }
    }

    if (sps) {
        // avc version(8bit), avc profile id(8bit) + avc compatibility(8bit) +
        // avc level(8bit) + reserved(6bit) + nalu_length_size_minus_one (2bit, 2) +
        // reserved(3bit) + sps_num(5bit) +
        //     loop(sps_num): sps size (16bit) + sps data
        // pps_num(8bit)
        //      loop(pps_num): pps size (16bit) + (00 00 00 01) pps data
        int pkt_sz = 5 + 1 + (sps ? 2 + (0 + sps->nalu_size) : 0) +
                         1 + (pps ? 2 + (0 + pps->nalu_size) : 0);

        if (sps->nalu_size < 4) {
            sp_error("sps size %lu", sps->nalu_size);
            return ERROR_H264_ENCODER;
        }

        uint8_t flag = H264;
        // 1 keyframe 2. inner 3. h263 disposable 4.5. frame info or order frame
        uint8_t frame_type = 0x01 << 4;
        flag |= frame_type;

        nalu_length_size_minus_one = 3;
        auto pkt = AVPacket::create_empty_cap(AVMessageType::AV_MESSAGE_DATA,
                         AV_STREAM_TYPE_VIDEO,
                         AVPacketType{AV_VIDEO_TYPE_SEQUENCE_HEADER},
                         pkt_sz,
                         0,
                         0,
                         flag,
                         H264);

        BitContext bc(pkt->buffer(), pkt->cap());
        bc.write_int8(0x01);  // avc version
        bc.write_int8(sps->nalu_buf[1]);  // avc profile
        bc.write_int8(0x00);  // avc compatibility
        bc.write_int8(sps->nalu_buf[3]);  // avc level
        bc.write_int8(nalu_length_size_minus_one);  // avc level

        bc.write_int8(1);  // 1 sps num
        bc.write_int16(sps->nalu_size);
        // bc.write_int32(0x01);  // nalu start code 00 00 00 01
        bc.write_bytes(sps->nalu_buf, sps->nalu_size);

        bc.write_int8(pps ? 1 : 0);
        if (pps) {
            bc.write_int16(pps->nalu_size);
            // bc.write_int32(0x01);  // nalu start code 00 00 00 01
            bc.write_bytes(pps->nalu_buf, pps->nalu_size);
        }

        pkt->skip(bc.pos() - pkt->buffer());
        pkts.push_back(pkt);

        sp_debug("avc seq header size %d, flag %x", pkt->size(), flag);
    }

    for (auto& n : ctx->nalu_list) {
        if (n.is_sps() || n.is_pps() || n.is_aud()) {
            continue;
        }

        if (nalu_length_size_minus_one < 0) {
            nalu_length_size_minus_one = 3;
        }

        uint8_t flag = H264;
        // 1 keyframe 2. inner 3. h263 disposable 4.5. frame info or order frame
        if (n.is_idr() || n.is_slice()) {
            uint8_t frame_type = (n.is_idr() ? 1 : 2) << 4;
            flag |= frame_type;
        } else if (n.is_sei()) {
            uint8_t frame_type = 0x5 << 4;
            flag |= frame_type;
        } else {
            continue;
        }

        // size + nalu start code + nalu
        size_t pkt_sz = nalu_length_size_minus_one + 1 + 0 + n.nalu_size;
        auto pkt = AVPacket::create_empty_cap(AVMessageType::AV_MESSAGE_DATA,
                                              AV_STREAM_TYPE_VIDEO,
                                              AVPacketType{AV_VIDEO_TYPE_DATA},
                                              pkt_sz,
                                              ctx->dts / ctx->timebase,
                                              ctx->pts / ctx->timebase,
                                              flag,
                                              H264);

        BitContext bc(pkt->buffer(), pkt->cap());
        bc.write_bits(n.nalu_size, 8 * (nalu_length_size_minus_one+1));
        // bc.write_int32(0x01);  // nalu start code
        bc.write_bytes(n.nalu_buf, n.nalu_size);

        pkt->skip(bc.pos() - pkt->buffer());
        pkts.push_back(pkt);
    }

    return SUCCESS;
}

error_t NALUParser::encode_annexb(NALUContext *ctx, PAVPacket &pkt) {
    return ERROR_H264_NOT_IMPL;
}

H264AVCodecParser::H264AVCodecParser() {
    nalu_parser = std::make_unique<NALUParser>();
}

error_t H264AVCodecParser::encode_avc(AVCodecContext* ctx, uint8_t* in_buf,
                                      int in_size, std::list<PAVPacket>& pkts) {

    NALUContext nalus(ctx->dts, ctx->pts, ctx->timebase);
    error_t ret = nalu_parser->decode(in_buf, in_size, &nalus, false);

    if (ret != SUCCESS) {
        return ret;
    }

    return nalu_parser->encode_avc(&nalus, pkts);
}

}
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

    sp_info("nalu type %x, sz %lu", nal_header.nal_unit_type, sz);
    return SUCCESS;
}

error_t NALUContext::append(uint8_t* buf, size_t sz) {
    H264NAL nal;
    error_t ret = nal.parse_nal(buf, sz);
    nalu_list.push_back(nal);
    return ret;
}

NALUParser::NALUParser(bool avc) {
    is_avc = avc;
    nalu_length_size_minus_one = -1;
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

error_t NALUParser::split_nalu(uint8_t* buf, size_t sz,
                               NALUContext* ctx,
                               bool is_seq_header) {
    if (is_seq_header) {
        return decode_header(buf, sz, ctx);
    }

    return decode(buf, sz, ctx);
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

            if ((ret = decode(bc.pos(), nalu_size, ctx)) != SUCCESS) {
                return ret;
            }
            bc.skip_bytes(nalu_size);
        }

        int pps_num = bc.read_int8();
        for (int i = 0; i < pps_num; ++i) {
            int nalu_size = bc.read_int16() + 2;
            if ((ret = bc.acquire(nalu_size)) != SUCCESS) {
                sp_error("fail acquire nalu size %d, ret %d", nalu_size, ret);
                return ERROR_H264_DECODER;
            }

            if ((ret = decode(bc.pos(), nalu_size, ctx)) != SUCCESS) {
                return ret;
            }
            bc.skip_bytes(nalu_size);
        }
    } else {
        is_avc = false;
        return decode(buf, sz, ctx);
    }

    return ret;
}

error_t NALUParser::decode(uint8_t* buf, size_t sz,
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
            bc.skip_bytes(start_pos - bc.pos() + 3);
        }

        if (bc.acquire(nalu_size) != SUCCESS) {
            sp_error("fail acquire nalu size %lu", nalu_size);
            return ERROR_H264_DECODER;
        }

        if ((ret = ctx->append(bc.pos(), nalu_size)) != SUCCESS) {
            return ret;
        }
        bc.skip_bytes(nalu_size);
    }

    return SUCCESS;
}

}
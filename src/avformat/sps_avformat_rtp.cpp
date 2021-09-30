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

#include <sps_avformat_rtp.hpp>
#include <sps_log.hpp>

namespace sps {

bool is_rtcp(int pt) {
    return pt >= RTP_SR && pt <= RTP_APP;
}

error_t RtpPayloadHeader::decode(BitContext &bc) {
    error_t ret = bc.acquire(12);

    if (ret != SUCCESS) {
        sp_error("fail require rtp header 12");
        return ret;
    }

    fix_header.version = bc.read_bits(2);
    fix_header.padding = bc.read_bits(1);
    fix_header.extension = bc.read_bits(1);
    fix_header.cc        = bc.read_bits(4);

    fix_header.mark      = bc.read_bits(1);
    fix_header.payload_type = bc.read_bits(7);

    fix_header.sequence_num = bc.read_bits(16);

    fix_header.timestamp    = bc.read_int32();
    fix_header.ssrc         = bc.read_int32();

    ret = bc.acquire(fix_header.cc * 4);
    if (ret != SUCCESS) {
        sp_error("fail require rtp csrc %d", fix_header.cc * 4);
        return ret;
    }

    for (int i = 0; i < fix_header.cc; ++i) {
        csrc.push_back(bc.read_int32());
    }

    if (fix_header.extension > 0 && (ret = decode_extension(bc)) != SUCCESS) {
        return ret;
    }

    sp_info("[rtp @header] pt=%d, ssrc=%x, seq=%d, time=%u %s",
            fix_header.payload_type, fix_header.ssrc,
            fix_header.sequence_num, fix_header.timestamp,
            fix_header.mark ? ", Mark" : "");

    return ret;
}

error_t RtpPayloadHeader::decode_extension(BitContext &bc) {
    error_t ret = bc.acquire(4);

    if (ret != SUCCESS) {
        sp_error("fail require rtp header extension 4");
        return ret;
    }

    extension_info.define_by_profile = bc.read_int16();
    extension_info.length = bc.read_int16();

    if (extension_info.length == 0) {
        return ret;
    }

    if ((ret = bc.acquire(extension_info.length)) != SUCCESS) {
        sp_error("fail require rtp header extension length %u",
                 extension_info.length);
        return ret;
    }

    extension_info.header_extension = std::make_unique<uint8_t[]>(extension_info.length);
    bc.read_bytes(extension_info.header_extension.get(), extension_info.length);

    return ret;
}

}
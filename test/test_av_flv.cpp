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
// Created by byrcoder on 2021/6/19.
//
#include <gtest/gtest.h>

#include <sps_avformat_flvdec.hpp>
#include <sps_avformat_flvenc.hpp>
#include <sps_url_file.hpp>
#include <sps_log.hpp>

extern const char* filename;
extern const char* out_filename;

namespace sps {

error_t test_flv() {
    auto in_file = std::make_shared<FileURLProtocol>();
    auto ret     = in_file->open(filename);

    if (ret != SUCCESS) {
        sp_error("open input filename failed ret: %d, %s", ret, filename);
        return ret;
    }

    auto out_file = std::make_shared<FileURLProtocol>(true, false);
    ret           = out_file->open(out_filename);

    if (ret != SUCCESS) {
        sp_error("open output filename failed ret: %d, %s", ret, out_filename);
        return ret;
    }

    FlvDemuxer flv_demuxer(in_file);
    PSpsAVPacket pkt;

    ret = flv_demuxer.read_header(pkt);
    if (ret != SUCCESS) {
        sp_error("fail read FLV HEADER ret: %d", ret);
        return ret;
    }

    FlvAVMuxer flv_muxer(out_file);
    ret = flv_muxer.write_header(pkt);

    pkt->debug();

    if (ret != SUCCESS) {
        sp_error("fail write FLV HEADER ret: %d", ret);
        return ret;
    }

    do {
        pkt.reset();
        ret = flv_demuxer.read_packet(pkt);

        if (ret != SUCCESS) {
            sp_error("fail read FLV PACKET %d", ret);
            break;
        }
        // pkt->debug();

        ret = flv_muxer.write_message(pkt);
        if (ret != SUCCESS) {
            sp_error("fail write FLV PACKET %d", ret);
            break;
        }

    } while(true);

    return ret == ERROR_IO_EOF ? SUCCESS : ret;
}

GTEST_TEST(FLV, DEMUX) {
    EXPECT_TRUE(test_flv() == SUCCESS);
}

}
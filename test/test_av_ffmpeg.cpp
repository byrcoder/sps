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
// Created by byrcoder on 2022/5/7.
//

#include <gtest/gtest.h>

#include <sps_avformat_ffmpeg_dec.hpp>
#include <sps_avformat_ffmpeg_enc.hpp>
#include <sps_io_url_file.hpp>
#include <sps_log.hpp>
#include <sps_auto_header.hpp>

#ifdef FFMPEG_ENABLED

using namespace sps;

extern const char* filename;
extern const char* out_filename;

GTEST_TEST(FFMPEG, DEMUX) {
    auto in_file = std::make_shared<FileURLProtocol>();
    auto ret     = in_file->open(filename);
    EXPECT_TRUE(ret == SUCCESS);

    if (ret != SUCCESS) {
        sp_error("open input filename failed ret: %d, %s", ret, filename);
        return;
    }

    FFmpegAVDemuxer ffmpeg_demuxer(in_file);
    PAVPacket pkt;

    ret = ffmpeg_demuxer.init();
    EXPECT_TRUE(ret == SUCCESS);

    if (ret != SUCCESS) {
        sp_error("open output filename failed ret: %d, %s", ret, filename);
        return;
    }

    auto out_file = std::make_shared<FileURLProtocol>(true, false);
    ret = out_file->open(out_filename);
    EXPECT_TRUE(ret == SUCCESS);

    if (ret != SUCCESS) {
        sp_error("open output filename failed ret: %d, %s", ret, out_filename);
        return;
    }

    auto url = std::make_shared<RequestUrl>();
    url->url = out_filename;

    FFmpegAVMuxer ffmpeg_muxer(out_file, url);

    ret = ffmpeg_muxer.init();
    EXPECT_TRUE(ret == SUCCESS);

    if (ret != SUCCESS) {
        sp_error("open output filename failed ret: %d, %s", ret, out_filename);
        return;
    }

    for (int i = 0; i < ffmpeg_demuxer.get_ctx()->nb_streams; i++) {
        EXPECT_TRUE(ffmpeg_muxer.on_av_stream(ffmpeg_demuxer.get_ctx()->streams[i]) == SUCCESS);
    }

    sp_trace("nbstream %d", ffmpeg_demuxer.get_ctx()->nb_streams);
    //输出一下格式------------------
    av_dump_format(ffmpeg_muxer.get_ctx(), 0, out_filename, 1);

    ret = ffmpeg_muxer.write_header(pkt);
    EXPECT_TRUE(ret == SUCCESS);

    if (ret != SUCCESS) {
        sp_error("write header output filename failed ret: %d, %s", ret, out_filename);
        return;
    }

    int n = 1000;

    do {
        pkt.reset();
        ret = ffmpeg_demuxer.read_packet(pkt);

        EXPECT_TRUE(ret == SUCCESS);

        if (ret != SUCCESS) {
            sp_error("fail read ffmpeg PACKET %d", ret);
            break;
        }

        ret = ffmpeg_muxer.write_packet(pkt);

        EXPECT_TRUE(ret == SUCCESS);

        if (ret != SUCCESS) {
            sp_error("fail read ffmpeg PACKET %d", ret);
            break;
        }

    } while(--n);

    EXPECT_TRUE(ret == SUCCESS);
}

#endif

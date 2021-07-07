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
// Created by byrcoder on 2021/6/30.
//
#include <gtest/gtest.h>

#include <sps_avformat_rtmpdec.hpp>
#include <librtmp/sps_librtmp.hpp>
#include <sps_url_rtmp.hpp>
#include <sps_url_protocol.hpp>
#include <sps_sync.hpp>

using namespace sps;

GTEST_TEST(RTMP_CIENT, CREATE) {
    auto& pf = SingleInstance<UrlProtocol>::get_instance();

    std::string url = "rtmp://pull-flv-f11-admin.douyincdn.com/stage/stream-685534728341684325_md"; //rtmp://127.0.0.1/live/test?app=live&tcUrl=baidu.com/live";
    auto url_rtmp = pf.create(url);

    EXPECT_TRUE(url_rtmp->open(url) == SUCCESS);

    RtmpDemuxer demuxer(url_rtmp);

    PSpsAVPacket buffer;
    while (demuxer.read_packet(buffer) == SUCCESS) {
        ;
    }

    SingleInstance<Sleep>::get_instance().sleep(4);
}

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
// Created by byrcoder on 2021/9/28.
//

#include <gtest/gtest.h>

#include <sps_avcodec_parser.hpp>
#include <sps_avformat_flvenc.hpp>
#include <sps_avformat_rtpdec.hpp>

#include <sps_url_file.hpp>

#include <sps_st_io_udp.hpp>
#include <sps_log.hpp>
#include <sps_co.hpp>

using namespace sps;

const char* sps_pps =                    "01 64 00 1e ff e1 00 1d 67"
                            " 64 00 1e ac d1 00 c8 3b f8 89 a8 28 30 32 00 00"
                            " 03 00 02 00 00 03 00 61 1e 2c 5a 24 01 00 05 68"
                            " eb e1 1c b0";

uint8_t to_int(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    }

    if (c >= 'a' && c <= 'f') {
        return 10 + c - 'a';
    }

    if (c >= 'A' && c <= 'F') {
        return 10 + c - 'A';
    }

    return 0;
}

uint8_t to_hex(char h, char l) {
    return to_int(h) << 4 | to_int(l);
}

uint8_t* load_sps_pps(const char* sps_pps, int& size) {
    const char* p = sps_pps;

    uint8_t  max_len = strlen(sps_pps);
    uint8_t* buf     = (uint8_t*) malloc(sizeof(uint8_t) * max_len);
    int      i       = 0;

    while (*p) {
        if (*p == ' ') {
            ++p;
            continue;
        }

        buf[i++] = to_hex(*p, *(p+1));
        p += 2;
        printf("%2x ", buf[i-1]);
    }
    size = i;

    printf(" size=%x\r\n", size);

    return buf;
}

GTEST_TEST(RTP, H264) {

    PUdpFd fd;
    auto ret = UdpFd::create_fd(9000, fd);

    EXPECT_TRUE(ret == SUCCESS);

    if (ret != SUCCESS) {
        return;
    }

    // 9001 port not used
    auto sock = std::make_shared<StUdpClientSocket>("127.0.0.1", 9001, fd);
    sock->set_recv_timeout(10 * 1000 * 1000);

    RtpDemuxer rtp(sock);
    rtp.init_codec(AVCODEC_H264);

    auto io = std::make_shared<FileURLProtocol>(true, false);
    if ((ret = io->open("rtp_to_flv.flv")) != SUCCESS) {
        sp_error("open file fail ret %d", ret);
        EXPECT_TRUE(ret == SUCCESS);
        return;
    }
    auto flv_muxer = std::make_shared<FlvAVMuxer>(io);
    PAVPacket flv_header;
    flv_muxer->write_header(flv_header);

    {
        int size = 0;
        uint8_t *seq_header = load_sps_pps(sps_pps, size);
        auto h = AVPacket::create(AV_MESSAGE_DATA, AV_STREAM_TYPE_VIDEO,
                         AVPacketType {0},
                         seq_header, size, 0, 0,
                         0x17, 7);
        flv_muxer->write_message(h);
    }


    PAVPacket pkt;
    while ((ret = rtp.read_packet(pkt)) == SUCCESS) {
        ret = flv_muxer->write_message(pkt);

        EXPECT_TRUE(ret == SUCCESS);
        // pkt->debug();
    }

    sp_info("rtp decode fail ret %d", ret);
    EXPECT_TRUE(ret == ERROR_SOCKET_TIMEOUT);
}

GTEST_TEST(RTP, AAC) {

    PUdpFd fd;
    auto ret = UdpFd::create_fd(9000, fd);

    EXPECT_TRUE(ret == SUCCESS);

    if (ret != SUCCESS) {
        return;
    }

    // 9001 port not used
    auto sock = std::make_shared<StUdpClientSocket>("127.0.0.1", 9001, fd);
    sock->set_recv_timeout(10 * 1000 * 1000);

    RtpDemuxer rtp(sock);
    rtp.init_codec(AVCODEC_AAC);

    auto io = std::make_shared<FileURLProtocol>(true, false);
    if ((ret = io->open("rtp_to_aac.flv")) != SUCCESS) {
        sp_error("open file fail ret %d", ret);
        EXPECT_TRUE(ret == SUCCESS);
        return;
    }
    auto flv_muxer = std::make_shared<FlvAVMuxer>(io);
    PAVPacket flv_header;
    flv_muxer->write_header(flv_header);

    {
        int size = 0;
        uint8_t *seq_header = load_sps_pps(sps_pps, size);
        auto h = AVPacket::create(AV_MESSAGE_DATA, AV_STREAM_TYPE_VIDEO,
                                  AVPacketType {0},
                                  seq_header, size, 0, 0,
                                  0x17, 7);
        flv_muxer->write_message(h);
    }


    PAVPacket pkt;
    while ((ret = rtp.read_packet(pkt)) == SUCCESS) {
        ret = flv_muxer->write_message(pkt);

        EXPECT_TRUE(ret == SUCCESS);
        // pkt->debug();
    }

    sp_info("rtp decode fail ret %d", ret);
    EXPECT_TRUE(ret == ERROR_SOCKET_TIMEOUT);
}

GTEST_TEST(RTP, TS) {

    PUdpFd fd;
    auto ret = UdpFd::create_fd(9000, fd);

    EXPECT_TRUE(ret == SUCCESS);

    if (ret != SUCCESS) {
        return;
    }

    // 9001 port not used
    auto sock = std::make_shared<StUdpClientSocket>("127.0.0.1", 9001, fd);
    sock->set_recv_timeout(10 * 1000 * 1000);

    RtpDemuxer rtp(sock);

    auto io = std::make_shared<FileURLProtocol>(true, false);
    if ((ret = io->open("rtp_ts_to_flv.flv")) != SUCCESS) {
        sp_error("open file fail ret %d", ret);
        EXPECT_TRUE(ret == SUCCESS);
        return;
    }
    auto flv_muxer = std::make_shared<FlvAVMuxer>(io);
    PAVPacket flv_header;
    flv_muxer->write_header(flv_header);

    {
        int size = 0;
        uint8_t *seq_header = load_sps_pps(sps_pps, size);
        auto h = AVPacket::create(AV_MESSAGE_DATA, AV_STREAM_TYPE_VIDEO,
                                  AVPacketType {0},
                                  seq_header, size, 0, 0,
                                  0x17, 7);
        flv_muxer->write_message(h);
    }


    PAVPacket pkt;
    while ((ret = rtp.read_packet(pkt)) == SUCCESS) {
        ret = flv_muxer->write_message(pkt);

        EXPECT_TRUE(ret == SUCCESS);
        // pkt->debug();
    }

    sp_info("rtp decode fail ret %d", ret);
    EXPECT_TRUE(ret == ERROR_SOCKET_TIMEOUT);
}
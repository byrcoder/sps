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
// Created by byrcoder on 2021/9/24.
//

#include <gtest/gtest.h>
#include <sps_st_io_udp.hpp>
#include <sps_log.hpp>
#include <sps_co.hpp>

using namespace sps;

GTEST_TEST(UDP, CLIENT) {
    PUdpFd fd;
    UdpFd::create_fd(fd);
    StUdpClientSocket udp_client("127.0.0.1", 900, fd);
    error_t ret = EXIT_SUCCESS;

    for (int i = 0; i < 10; ++i) {
        std::string buffer = "hello" + std::to_string(i);
        EXPECT_TRUE((ret = udp_client.write((void*) buffer.c_str(), buffer.size())) == SUCCESS);
    }

}

class UdpMockServer : public ICoHandler {
 public:
    UdpMockServer(std::shared_ptr<StUdpServerSocket> server) : server(std::move(server)) {}

 public:
    error_t handler() override {
        while (true) {
            auto socket = server->accept();
        }
        return SUCCESS;
    }

 private:
    std::shared_ptr<StUdpServerSocket> server;
};

GTEST_TEST(UDP, SERVER) {
    auto server = std::make_shared<StUdpServerSocket>();
    error_t ret = server->listen("", 900, false, 100);
    EXPECT_TRUE(ret == SUCCESS);

    if (ret != SUCCESS) {
        return;
    }

    int i = 0;
    static char buf[4096];

    auto socket = server->accept();
    socket->set_recv_timeout(10 * 1000 * 1000);
    size_t nr = 0;

    auto udp_server = std::make_shared<UdpMockServer>(server);
    ICoFactory::get_instance().start(udp_server);

    while (socket->read(buf, sizeof(buf), nr) == SUCCESS) {
        sp_info("udp read %.*s", (int) nr, buf);
        EXPECT_TRUE(socket->write(buf, nr) == SUCCESS);
        st_sleep(1);
    }

}
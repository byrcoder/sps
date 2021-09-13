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
// Created by byrcoder on 2021/9/2.
//

#include <gtest/gtest.h>
#include <sps_io_bits.hpp>
#include <sps_log.hpp>
#include <sps_st_io_ssl.hpp>
#include <sps_st_io_tcp.hpp>

using namespace sps;

#ifdef OPENSSL_ENABLED

GTEST_TEST(SSL, CLIENT) {
    SSL_load_error_strings();

    OpenSSL_add_all_algorithms();
    SSL_library_init();
    auto ssl = SingleInstance<ClientSocketFactory>::get_instance().create_ss(
            Transport::HTTPS, "localhost", 444, 10 * 1000 * 1000);

    EXPECT_TRUE(ssl != nullptr);
    if (!ssl) {
        return;
    }

    sp_info("success handshake");
    const char* data = "GET / HTTP/1.1\r\nUser-Agent: curl/7.29.0\r\nHost: localhost\r\nAccept: */*\r\n\r\n";
    size_t len = strlen(data);
    EXPECT_TRUE(ssl->write((void*) data, len) == SUCCESS);
    sp_info("success write %lu request\r\n%s", len, data);

    char buf[1024];
    while (true) {
        size_t n = 0;
        if (ssl->read(buf, sizeof(buf), n) == SUCCESS) {
            sp_info("reading data n %d %.*s", (int) n, (int) n, buf);
            continue;
        }
        break;
    }
}

GTEST_TEST(SSL, SERVER) {
    SSL_load_error_strings();

    OpenSSL_add_all_algorithms();
    SSL_library_init();
    auto ssl = SingleInstance<ServerSocketFactory>::get_instance().create_ss(
            Transport::HTTPS);

    // ssl config init
    SSLConfig config;
    config.crt_file = "./cert/cert.pem";
    config.key_file = "./cert/key.pem";

    auto ssl_ss = dynamic_cast<StSSLServerSocket*>(ssl.get());
    ssl_ss->init_config(config);

    EXPECT_TRUE(ssl != nullptr);
    if (!ssl) {
        return;
    }

    EXPECT_TRUE(ssl->listen("", 445) == SUCCESS);

    const char* res = "HTTP/1.1 404 Not Found\r\n"
                      "Content-Type: text/plain\r\n"
                      "Content-Length: 9\n"
                      "Access-Control-Allow-Credentials: true\r\n"
                      "Access-Control-Allow-Origin: *\r\n"
                      "Connection: close\r\n\r\n"
                      "Not Found";

    int i = 0;
    do {
        auto sock = ssl->accept();
        EXPECT_TRUE(sock != nullptr);
        EXPECT_TRUE(sock->write((void*) res, strlen(res)) == SUCCESS);
    } while (i++ < 10);
}

#endif

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
#include <sps_sys_st_io_ssl.hpp>
#include <sps_sys_st_io_tcp.hpp>

using namespace sps;

#ifdef OPENSSL_ENABLED

GTEST_TEST(SSL, CLIENT) {
    SSL_load_error_strings();

    OpenSSL_add_all_algorithms();
    SSL_library_init();
    auto ssl = SingleInstance<ClientSocketFactory>::get_instance().create_ss(
            Transport::HTTPS, "www.baidu.com", 443, 10 * 1000 * 1000);

    EXPECT_TRUE(ssl != nullptr);
    if (!ssl) {
        return;
    }

    sp_info("success handshake");
    const char* data = "GET / HTTP/1.1\r\nUser-Agent: curl/7.29.0\r\nHost: www.baidu.com\r\nAccept: */*\r\n\r\n";
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
    config.crt_file = "../cert/cert.pem";
    config.key_file = "../cert/key.pem";

    auto ssl_ss = dynamic_cast<StSSLServerSocket*>(ssl.get());
    ssl_ss->init_config(config);

    EXPECT_TRUE(ssl != nullptr);
    if (!ssl) {
        return;
    }

    int ret = ssl->listen("", 445);
    EXPECT_TRUE(ret == SUCCESS);
    if (ret != SUCCESS) {
	return;
    }

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

GTEST_TEST(SSL, CLIENTBIO)
{
    BIO *sbio = NULL, *out = NULL;
    int len;
    char tmpbuf[1024];
    SSL_CTX *ctx;
    SSL *ssl;
    const char *connect_str = "www.baidu.com:443";
    int i = 0;
#if OPENSSL_VERSION_NUMBER >= 0x1010107f
    ctx = SSL_CTX_new(TLS_client_method());
#else
    ctx = SSL_CTX_new(TLSv1_2_client_method());
#endif    

    /*
     * We'd normally set some stuff like the verify paths and * mode here
     * because as things stand this will connect to * any server whose
     * certificate is signed by any CA.
     */

    sbio = BIO_new_ssl_connect(ctx);

    BIO_get_ssl(sbio, &ssl);

    if (!ssl) {
        sp_error("Can't locate SSL pointer\n");
        goto end;
    }

    /* Don't want any retries */
    SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);
#if OPENSSL_VERSION_NUMBER >= 0x1010107f
    SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, NULL);
    SSL_set_tlsext_host_name(ssl, "www.baidu.com");
#else
#endif

    /* We might want to do other things with ssl here */

    BIO_set_conn_hostname(sbio, connect_str);

    out = BIO_new_fp(stdout, BIO_NOCLOSE);
    if (BIO_do_connect(sbio) <= 0) {
        sp_error("Error connecting to server\n");
        goto end;
    }

    if (BIO_do_handshake(sbio) <= 0) {
        sp_error("Error establishing SSL connection\n");
        goto end;
    }

    /* Could examine ssl here to get connection info */

    BIO_puts(sbio, "GET / HTTP/1.0\r\n\r\n");
    sp_trace("bio writing");
    for (; i < 1; ++i) {
        len = BIO_read(sbio, tmpbuf, 1024);
        if (len <= 0)
            break;
        sp_trace("bio read %d", len);
        BIO_write(out, tmpbuf, len);
        sp_trace( "bio write %d", len);
    }
    end:
    BIO_free_all(sbio);
    BIO_free(out);
}

#endif

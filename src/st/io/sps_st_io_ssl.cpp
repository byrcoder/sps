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
// Created by byrcoder on 2021/8/31.
//

#include <sps_st_io_ssl.hpp>
#include <sps_log.hpp>

#ifdef OPENSSL_ENABLED

namespace sps {

StSSLSocket::StSSLSocket(PIReaderWriter io) {
    this->io = std::move(io);

    // ssl init
#if (OPENSSL_VERSION_NUMBER < 0x10002000L) // v1.0.2
    ctx = SSL_CTX_new(TLSv1_2_method());
#else
    ctx = SSL_CTX_new(TLS_method());
#endif
    ssl = SSL_new(ctx);
    rbio = BIO_new(BIO_s_mem());
    wbio = BIO_new(BIO_s_mem());
    SSL_set_bio(ssl, rbio, wbio);
}

StSSLSocket::~StSSLSocket() {
    SSL_free(ssl);
    SSL_CTX_free(ctx);

    BIO_free(rbio);
    BIO_free(wbio);
}

void StSSLSocket::set_recv_timeout(utime_t tm) {
    io->set_recv_timeout(tm);
}

utime_t StSSLSocket::get_recv_timeout() {
    return io->get_recv_timeout();
}

error_t StSSLSocket::read_fully(void* buf, size_t size, ssize_t* nread) {
    size_t n = 0;
    error_t ret = SUCCESS;

    while (n < size) {
        size_t len = 0;
        if ((ret = read(buf, size-n, len)) != SUCCESS) {
            break;
        }

        n += len;
    }

    if (nread) {
        *nread = n;
    }

    return ret;
}

error_t StSSLSocket::read(void* buf, size_t size, size_t& nread) {
    // TODO: IMPL SSL
    error_t ret = SUCCESS;

    do {
        int n = SSL_read(ssl, buf, size);

        if (n > 0) {
            nread = n;
            break;
        }

        if (n == -1 && SSL_ERROR_WANT_READ == SSL_get_error(ssl, n)) {
            if ((ret = io->read(buf, size, nread)) != SUCCESS) {
                sp_error("fail ssl raw read ret %d", ret);
                return ret;
            }

            n = BIO_write(rbio, buf, nread);
            if (n <= 0) {
                sp_error("bio write failed n %d, num %ld", n, nread);
                return ERROR_SSL_READ;
            }
        } else {
            sp_error("ssl read failed n %d", n);
            return ERROR_SSL_READ;
        }
    } while (true);

    return ret;
}

void StSSLSocket::set_send_timeout(utime_t tm) {
    io->set_send_timeout(tm);
}

utime_t StSSLSocket::get_send_timeout() {
    return io->get_send_timeout();
}

error_t StSSLSocket::write(void* buf, size_t size) {
    // TODO: IMPL SSL
}

StSSLServerSocket::StSSLServerSocket(PIServerSocket ssock) {
    this->ssock = ssock;
}

error_t StSSLServerSocket::listen(std::string sip, int sport, bool reuse_sport, int back_log) {
    return SUCCESS;
}

PSocket StSSLServerSocket::accept() {
    auto skt = ssock->accept();
    // TODO: IMPL SSL
}

}

#endif

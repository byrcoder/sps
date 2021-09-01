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

#ifndef SPS_ST_IO_SSL_HPP
#define SPS_ST_IO_SSL_HPP

#include <sps_io_socket.hpp>

#ifdef OPENSSL_ENABLED

#include <openssl/ssl.h>

namespace sps {

/**
 * ssl wrapped
 */
class StSSLSocket : public IReaderWriter {
 public:
    explicit StSSLSocket(PIReaderWriter io);
    ~StSSLSocket();

 public:
    // reader
    void    set_recv_timeout(utime_t tm) override;
    utime_t get_recv_timeout() override;

    error_t read_fully(void* buf, size_t size, ssize_t* nread) override;
    error_t read(void* buf, size_t size, size_t& nread) override;

 public:
    // writer
    void set_send_timeout(utime_t tm) override;
    utime_t get_send_timeout() override;
    error_t write(void* buf, size_t size) override;

 private:
    PIReaderWriter io;

    // ssl context
    BIO* rbio;
    BIO* wbio;
    SSL_CTX* ctx;
    SSL* ssl;
};

/**
 * ssl server wrapped
 */
class StSSLServerSocket : public IServerSocket {
 public:
    StSSLServerSocket(PIServerSocket ssock);

 public:
    error_t listen(std::string sip, int sport, bool reuse_sport, int back_log) override;
    PSocket accept() override;

 private:
    PIServerSocket ssock;
};

}  // namespace sps

#endif

#endif  // SPS_ST_IO_SSL_HPP

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

#ifdef OPENSSL_ENABLED

namespace sps {

StSSLSocket::StSSLSocket(PIReaderWriter io) {
    this->io = std::move(io);
}

void StSSLSocket::set_recv_timeout(utime_t tm) {
    io->set_recv_timeout(tm);
}

utime_t StSSLSocket::get_recv_timeout() {
    return io->get_recv_timeout();
}

error_t StSSLSocket::read_fully(void* buf, size_t size, ssize_t* nread) {
    // TODO: IMPL SSL
}

error_t StSSLSocket::read(void* buf, size_t size, size_t& nread) {
    // TODO: IMPL SSL
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

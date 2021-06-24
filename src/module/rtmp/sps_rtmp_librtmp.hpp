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

#ifndef SPS_RTMP_LIBRTMP_HPP
#define SPS_RTMP_LIBRTMP_HPP

#include <rtmp.h>
#include <sps_io_socket.hpp>

namespace sps {

void librtmp_init_once();

class LibRTMPHooks {
 public:
    LibRTMPHooks(PSocket io);
    ~LibRTMPHooks();

 public:
    RTMP* get_rtmp() { return rtmp; }

 public:
    error_t  get_error() { return error; }

 private:
    PSocket   skt;
    RTMP*     rtmp;
    RTMP_HOOK hook;
    error_t   error;

 public:
    // connect
    static int RTMP_Connect(RTMP *r, RTMPPacket *cp); // RTMP_CONNECT, FALSE=0 TRUE=1
    static int RTMP_TLS_Accept(RTMP *r, void *ctx);

    // send ret < 0 fail, n byte return ok
    static int RTMPSockBuf_Send(RTMPSockBuf *sb, const char *buf, int len);

    // read
    // return -1 error, n nbyte return success
    static int RTMPSockBuf_Fill(RTMPSockBuf *sb, char* buf, int nb_bytes);

    // close
    static int RTMPSockBuf_Close(RTMPSockBuf *sb);

    // search
    static int RTMP_IsConnected(RTMP *r);
    static int RTMP_Socket(RTMP *r);
};

}

#endif  // SPS_RTMP_LIBRTMP_HPP
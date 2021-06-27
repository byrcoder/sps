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

#ifndef SPS_RTMP_SERVER_HANDLER_HPP
#define SPS_RTMP_SERVER_HANDLER_HPP

#include <sps_host_phase_handler.hpp>
#include <sps_rtmp_librtmp.hpp>
#include <librtmp/rtmp.h>

namespace sps {

// rtmp handler
class RtmpServerHandler : public IPhaseHandler {
 public:
    RtmpServerHandler();

 public:
    error_t handler(ConnContext &ctx) override;
};

class RtmpServer404Handler : public IPhaseHandler {
 public:
    RtmpServer404Handler();

 public:
    error_t handler(ConnContext &ctx) override;
};

// rtmp handshake and connect
class RtmpPrepareHandler : public IPhaseHandler {
 public:
    RtmpPrepareHandler();

 public:
    error_t handler(ConnContext& ctx) override;
};

// rtmp server handshake
class RtmpServerHandshake {
 public:
    explicit RtmpServerHandshake(LibRTMPHooks* r);
    error_t handshake();

 private:
    LibRTMPHooks*     hook;
};

// rtmp server connect
class RtmpPreRequest {
 public:
    RtmpPreRequest(LibRTMPHooks* r);

 public:
    error_t connect();

 public:
    LibRTMPHooks*      hook;
    std::string       tc_url;
    std::string       stream_params;
    double            txn;
};

}

#endif  // SPS_RTMP_SERVER_HANDLER_HPP
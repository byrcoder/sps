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

#include <librtmp/rtmp.h>

#include <sps_host_phase_handler.hpp>
#include <librtmp/sps_librtmp.hpp>
#include <librtmp/sps_librtmp_packet.hpp>

namespace sps {

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
    explicit RtmpServerHandshake(RtmpHook* r);
    error_t handshake();

 private:
    RtmpHook*     hook;
};

// rtmp server connect
class RtmpPreRequest {
 public:
    explicit RtmpPreRequest(RtmpHook* r);

 public:
    error_t connect();

 protected:
    error_t on_recv_create_stream(CreateStreamRtmpPacket* p) const;
    error_t on_recv_play(PlayRtmpPacket* p);
    error_t on_recv_publish(PublishRtmpPacket* p);
    error_t on_recv_fcpublish(FCPublishRtmpPacket* p) const;
    error_t on_recv_release_stream(ReleaseStreamRtmpPacket* p) const;

 public:
    RtmpHook*         hook;
    std::string       tc_url;
    std::string       stream_params;
    double            txn = 0;
    bool              playing = false;
    bool              publishing = false;
};

}

#endif  // SPS_RTMP_SERVER_HANDLER_HPP
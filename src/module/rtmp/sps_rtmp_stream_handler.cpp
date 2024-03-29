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
// Created by byrcoder on 2021/7/4.
//

#include <sps_rtmp_stream_handler.hpp>

#include <list>
#include <memory>
#include <string>
#include <utility>

#include <sps_avformat_rtmpdec.hpp>
#include <sps_avformat_rtmpenc.hpp>
#include <sps_avformat_ffmpeg_dec.hpp>
#include <sps_avformat_ffmpeg_enc.hpp>

#include <sps_avformat_librtmp_packet.hpp>
#include <sps_rtmp_server.hpp>
#include <sps_rtmp_server_handler.hpp>

#include <sps_stream.hpp>
#include <sps_stream_handler.hpp>

#include <sps_io_url_rtmp_ffmpeg.hpp>

namespace sps {

RtmpServerStreamHandler::RtmpServerStreamHandler()
    : IPhaseHandler("rtmp-server") {
}

error_t RtmpServerStreamHandler::handler(IConnection &c) {
    auto&       ctx      = *dynamic_cast<RtmpContext*>(c.get_context().get());
#ifdef FFMPEG_ENABLED
    auto        io       = std::make_shared<FFmpegRtmpUrlProtocol>(ctx.hk);
    auto        sh       = std::make_shared<StreamHandler>(io, ctx.publishing);
    auto        flv_url  = "http://" + ctx.req->host + ctx.req->path + ".flv";
    if (!ctx.req->params.empty()) {
        flv_url += "?" + ctx.req->params;
    }
    PRequestUrl flv_req;

    RequestUrl::from(flv_url, flv_req);
    ctx.req     = flv_req;
    return sh->handler(c);
#else
    ctx.req->ext = "-";
    auto    io   = std::make_shared<RtmpUrlProtocol>(ctx.hk);
    auto    sh   = std::make_shared<StreamHandler>(io, ctx.publishing);
    return sh->handler(c);
#endif
}

}  // namespace sps

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
// Created by byrcoder on 2021/6/22.
//

#include <sps_rtmp_handler.hpp>

namespace sps {

// TODO: IMPL
RtmpServerHandler::RtmpServerHandler() : IPhaseHandler("rtmp-server") {
}

error_t RtmpServerHandler::handler(HostPhaseCtx &ctx) {
    return ERROR_RTMP_NOT_IMPL;
}

RtmpServer404Handler::RtmpServer404Handler() : IPhaseHandler("rtmp-404-handler") {
}

error_t RtmpServer404Handler::handler(HostPhaseCtx &ctx) {
    sp_error("host not found host: %s", ctx.req->get_host());
    return ERROR_UPSTREAM_NOT_FOUND;
}

}
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
// Created by byrcoder on 2021/6/15.
//

#include <sps_http_adapter_phase_handler.hpp>

namespace sps {

HttpAdapterPhaseHandler::HttpAdapterPhaseHandler() : IPhaseHandler("adapter-handler") {
    proxy_handler  = std::make_unique<HttpProxyPhaseHandler>();
    stream_handler = std::make_unique<StreamPhaseHandler>();
}

error_t HttpAdapterPhaseHandler::handler(HostPhaseCtx &ctx) {
    if (ctx.host->stream_module) {
        return stream_handler->handler(ctx);
    } else {
        return proxy_handler->handler(ctx);
    }
}

}
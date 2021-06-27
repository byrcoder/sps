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

#include <sps_rtmp_server.hpp>

namespace sps {

RtmpConnHandler::RtmpConnHandler(PSocket io, PServerPhaseHandler& handler) :
        IConnHandler(std::move(io)), hd(handler) {
    hk = std::make_unique<LibRTMPHooks>(this->io);
}

error_t RtmpConnHandler::handler() {
    ConnContext ctx(nullptr, io, this);
    do {
        error_t ret = SUCCESS;

        if ((ret = hd->handler(ctx)) != SUCCESS) {
            return ret;
        }
        sp_trace("success handler ret %d", ret);
    } while(true);

    return SUCCESS;
}

RtmpConnHandlerFactory::RtmpConnHandlerFactory(PServerPhaseHandler hd) {
    handler = std::move(hd);
}

PIConnHandler RtmpConnHandlerFactory::create(PSocket io) {
    return std::make_shared<RtmpConnHandler>(io, handler);
}

}
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
// Created by byrcoder on 2021/8/24.
//

#include <sps_srt_server.hpp>

#ifndef SRT_DISABLED

namespace sps {

SrtConnHandler::SrtConnHandler(PSocket io, PServerPhaseHandler& handler) :
        IConnHandler(std::move(io)), hd(handler) {
}

error_t SrtConnHandler::handler() {
    ConnContext ctx(nullptr, io, this);
    do {
        error_t ret = SUCCESS;

        if ((ret = hd->handler(ctx)) != SUCCESS) {
            return ret;
        }
        sp_trace("success handler ret %d", ret);
    } while (true);

    return SUCCESS;
}

SrtConnHandlerFactory::SrtConnHandlerFactory(PServerPhaseHandler hd) {
    handler = std::move(hd);
}

PIConnHandler SrtConnHandlerFactory::create(PSocket io) {
    return std::make_shared<SrtConnHandler>(io, handler);
}

}  // namespace sps

#endif

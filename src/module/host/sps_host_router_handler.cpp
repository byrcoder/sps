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

#include <sps_host_router_handler.hpp>

#include <utility>

namespace sps {

HostRouterPhaseHandler::HostRouterPhaseHandler(
        PHostModulesRouter router,
        PIPhaseHandler df, PIPhaseHandler not_found_handler) :
          IPhaseHandler("host-router-handler") {
    this->router          = std::move(router);
    this->host_handler    = std::move(df);
    this->not_found_handler = std::move(not_found_handler);
}

error_t HostRouterPhaseHandler::handler(ConnContext &ctx) {
    if (!ctx.req) {
        sp_error("host is empty");
    } else {
        ctx.host = router->find_host(ctx.req->host);
    }

    if (!ctx.host) {
        sp_error("Not found host:%s", ctx.req ? ctx.req->host.c_str() : "");

        if (not_found_handler) {
            return not_found_handler->handler(ctx);
        }
        return ERROR_HOST_NOT_EXISTS;
    }

    return host_handler->handler(ctx);
}

}  // namespace sps

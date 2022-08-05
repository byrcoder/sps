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
// Created by byrcoder on 2021/7/16.
//

#include <sps_http_stream_phase_handler.hpp>

#include <sps_http_socket.hpp>
#include <sps_stream_handler.hpp>

namespace sps {

std::string HttpStreamPhaseHandler::get_cache_key(PRequestUrl& req) {
    std::string path   = req->get_path();
    std::string url    = req->host + (req->ext.empty() ? path
            : path.substr(0, path.size()-req->ext.size()-1));
    return url;
}

HttpStreamPhaseHandler::HttpStreamPhaseHandler()
    : IPhaseHandler("http-stream-handler") {
}

error_t HttpStreamPhaseHandler::handler(ConnContext& ctx) {
    error_t ret  = SUCCESS;
    auto    rsp  = std::make_shared<HttpResponseSocket>(ctx.socket, ctx.ip, ctx.port);
    auto    sh   = std::make_shared<StreamHandler>(rsp, ctx.req->method == "POST");

    rsp->init(200, nullptr, -1, false);
    ret = sh->handler(ctx);

    if (ret == ERROR_STREAM_PUBLISH_NOT_SUPPORT) {
        rsp->init(403, nullptr, -1, false);
        ret = rsp->write_header();
        sp_error("not support proxy for streaming! http rsp %d, ret %d", 403, ret);
        return ret;
    } else if (ret == ERROR_STREAM_SOURCE_NOT_EXITS) {
        rsp->init(404, nullptr, -1, false);
        ret = rsp->write_header();
        sp_error("not publishing or edge for streaming! url %s http rsp %d, ret %d, method %s",
                 ctx.req->get_url(), 404, ret, ctx.req->get_method());
        return ret;
    }

    return ret;
}

}  // namespace sps

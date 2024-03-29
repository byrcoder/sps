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

HttpStreamPhaseHandler::HttpStreamPhaseHandler()
    : IPhaseHandler("http-stream-http_server") {
}

error_t HttpStreamPhaseHandler::handler(IConnection& c) {
    error_t ret  = SUCCESS;
    auto&   ctx  = *dynamic_cast<HostContext*> (c.get_context().get());
    auto    sock = std::make_shared<HttpRequestSocket>(
                       c.socket, ctx.req->ip, ctx.req->port,
                       ctx.req->is_chunked(), ctx.req->get_content_length());
    auto    rsp  = std::make_shared<HttpResponseSocket>(sock, sock->get_peer_ip(),
                                                        sock->get_peer_port());
    auto    sh   = std::make_shared<StreamHandler>(rsp, ctx.req->method == "POST");

    rsp->init(200, nullptr, -1, false);
    ret = sh->handler(c);

    if (ret == ERROR_STREAM_PUBLISH_NOT_SUPPORT) {
        rsp->init(403, nullptr, -1, false);
        ret = rsp->write_header();
        sp_warn("Response %s:%d not-support %s-%s-%s status %d %d",
                ctx.req->ip.c_str(), ctx.req->port,
                ctx.req->get_host(), ctx.req->get_url(), ctx.req->get_params(), 403, ret);
        return ret;
    } else if (ret == ERROR_STREAM_SOURCE_NOT_EXITS) {
        rsp->init(404, nullptr, -1, false);
        ret = rsp->write_header();
        sp_trace("Response not-exits %s:%d %s-%s-%s status %d %d",
                 ctx.req->ip.c_str(), ctx.req->port,
                 ctx.req->get_host(), ctx.req->get_url(), ctx.req->get_params(), 404, ret);
        return ret;
    }

    return ret;
}

}  // namespace sps

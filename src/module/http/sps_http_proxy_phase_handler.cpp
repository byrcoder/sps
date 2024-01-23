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

#include <sps_http_proxy_phase_handler.hpp>

#include <memory>

#include <sps_http_parser.hpp>
#include <sps_http_server.hpp>
#include <sps_http_socket.hpp>

#include <sps_io_url_protocol.hpp>

namespace sps {

HttpProxyPhaseHandler::HttpProxyPhaseHandler()
    : IPhaseHandler("http-router-http_server") {
}

error_t HttpProxyPhaseHandler::handler(IConnection &c) {
    error_t  ret           = SUCCESS;
    auto&    ctx           = *dynamic_cast<HttpContext*> (c.get_context().get());
    auto&    host_ctx      = ctx.host;
    auto     proxy_req     = std::make_shared<RequestUrl>(*ctx.req);
    auto&    protocols     = SingleInstance<UrlProtocol>::get_instance();
    proxy_req->port        = 80;
    auto sock              = std::make_shared<HttpRequestSocket>(
                              c.socket, ctx.req->ip, ctx.req->port,
                              ctx.req->is_chunked(), ctx.req->get_content_length());
    ctx.host->get_proxy_info(proxy_req->ip, proxy_req->port);

    auto     url_protocol  = protocols.create(proxy_req);
    if (!url_protocol) {
        sp_error("not found protocol %s for proxy", proxy_req->schema.c_str());
        return ERROR_UPSTREAM_NOT_FOUND;
    }

    if ((ret = url_protocol->open(proxy_req, DEFAULT)) != SUCCESS) {
        sp_error("Failed open url protocol %s. %s:%d, ret:%d.",
                 proxy_req->url.c_str(), proxy_req->ip.c_str(), proxy_req->port, ret);
        return ret;
    }

    auto http_rsp = std::dynamic_pointer_cast<HttpResponse>(url_protocol->response());
    HttpResponseSocket rsp(sock, ctx.req->ip, ctx.req->port);
    rsp.init(http_rsp->status_code, &http_rsp->headers, http_rsp->content_length, http_rsp->chunked);

    if ((ret = rsp.write_header()) != SUCCESS) {
        sp_error("write head status: %d, %lu, %d, %d.", http_rsp->status_code,
                 http_rsp->headers.size(), http_rsp->content_length, ret);
        return ret;
    }
    sp_debug("write head status %d, %lu, %d.", http_rsp->status_code,
             http_rsp->headers.size(), http_rsp->content_length);

    char   buf[1024];
    int    len  = sizeof(buf);
    size_t nr   = 0;

    while ((ret = url_protocol->read(buf, len, nr)) == SUCCESS) {
        ret = rsp.write(buf, nr);
        if (ret != SUCCESS) {
            sp_error("failed write url protocol %d.", ret);
            return ret;
        }
    }

    sp_trace("final response code:%d, ret:%d, eof:%d, chunked:%d",
             http_rsp->status_code, ret, ret == ERROR_HTTP_RES_EOF, http_rsp->chunked);

    return (ret == ERROR_HTTP_RES_EOF || ret == SUCCESS) ?
                SPS_PHASE_SUCCESS_NO_CONTINUE : ret;
}

}  // namespace sps

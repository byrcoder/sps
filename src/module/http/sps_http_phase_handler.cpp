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

#include <sps_http_phase_handler.hpp>
#include <sps_http_socket.hpp>
#include <sps_http_server.hpp>
#include <log/sps_log.hpp>
#include <sps_io_url_protocol.hpp>
#include <memory>

namespace sps {

HttpParsePhaseHandler::HttpParsePhaseHandler()
    : IPhaseHandler("http-parser-http_server") {
}

error_t HttpParsePhaseHandler::handler(IConnection &c) {
    auto http_parser = std::make_shared<HttpParser>();
    error_t ret      = SUCCESS;

    if ((ret = http_parser->parse_header(c.socket,
                                      HttpType::REQUEST)) <= SUCCESS) {
        sp_debug("failed get http request ret:%d", ret);
        return ret;
    }

    auto req  = http_parser->get_request();
    auto ctx = std::make_shared<HttpContext>(req);
                                                                                 ;
    c.set_context(ctx);

    sp_trace("Request %s:%d %s %s %s %s, chunked %d, content-length %d",
             req->ip.c_str(), req->port, req->get_method(),
             req->host.c_str(), req->url.c_str(), req->params.c_str(),
             req->is_chunked(), req->get_content_length());

    return SPS_PHASE_CONTINUE;
}

Http404PhaseHandler::Http404PhaseHandler()
    : IPhaseHandler("http-404-http_server") {
}

error_t Http404PhaseHandler::handler(IConnection& ctx) {
    auto socket = ctx.socket;
    PHttpResponseSocket http_socket = std::make_shared<HttpResponseSocket>(
            socket,
            socket->get_peer_ip(),
            socket->get_peer_port());

    if (!http_socket) {
        sp_error("Fatal not http socket type(socket):%s", typeid(ctx.socket.get()).name());
        return ERROR_HTTP_SOCKET_CREATE;
    }

    http_socket->init(404, nullptr, 0, false);
    auto ret = http_socket->write_header();

    sp_trace("Response http 404 %d", ret);
    return SPS_PHASE_SUCCESS_NO_CONTINUE;
}

}  // namespace sps

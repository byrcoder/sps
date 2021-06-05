#include <app/http/http_phase_handler.hpp>
#include <app/http/http_socket.hpp>
#include <log/log_logger.hpp>
#include <app/url/url_protocol.hpp>
#include <memory>

namespace sps {

HttpPhCtx::HttpPhCtx(PRequestUrl r, PSocket s) {
    req      = std::move(r);
    socket   = std::move(s);
}

error_t HttpParsePhaseHandler::handler(HttpPhCtx &ctx) {
    auto http_parser = std::make_shared<HttpParser>();
    error_t ret = SUCCESS;

    sp_trace("Request Http Parse");

    if ((ret = http_parser->parse_header(ctx.socket, HttpType::REQUEST)) <= SUCCESS) {
        return ret;
    }

    ctx.req = http_parser->get_request();
    sp_trace("Request %s, %s, %s", ctx.req->host.c_str(), ctx.req->url.c_str(), ctx.req->params.c_str());

    return SPS_HTTP_PHASE_CONTINUE;
}

const char * HttpParsePhaseHandler::get_name() {
    return "http-parser-handler";
}

error_t HttpProxyPhaseHandler::handler(HttpPhCtx &ctx) {
    error_t ret   = SUCCESS;
    auto&   upf   = SingleInstance<UrlProtocol>::get_instance();
    auto    up    = upf.create(ctx.req);

    if (!up) {
        sp_error("Failed create url protocol %s, %s.",
                ctx.req->get_schema(), ctx.req->url.c_str());
        return SPS_HTTP_PHASE_ERROR;
    }

    HttpResponseSocket rsp(ctx.socket, ctx.ip, ctx.port);

    auto proxy_req  = std::make_shared<RequestUrl>();
    *proxy_req      = *ctx.req;
    auto path       = proxy_req->path;

    proxy_req->erase_header("Referer");

    if (ctx.session_host.empty()) {
        auto nh = path.find_first_of('/', 1);

        if (nh == std::string::npos) {
            sp_error("Failed open url protocol %s.", proxy_req->url.c_str());
            return ERROR_PROXY_PROTOCOL_HEADER_PARSE_FAILED;
        } else {
            proxy_req->host = path.substr(1, nh - 1);
            proxy_req->url =
                    path.substr(nh) + (ctx.req->params.empty() ? "" : "?" + ctx.req->params);
            proxy_req->port = 80;
            proxy_req->ip = "";
            proxy_req->headers.clear();

            ctx.session_host = proxy_req->host;
        }
    } else {
        proxy_req->host = ctx.session_host;
        proxy_req->url = path + (ctx.req->params.empty() ? "" : "?" + ctx.req->params);
    }

    if ((ret = up->open(proxy_req)) != SUCCESS && ret != ERROR_URL_RSP_NOT_INVALID) {
        sp_error("Failed open url protocol %s.", proxy_req->url.c_str());
        return ret;
    }

    PHttpResponse http_rsp = std::dynamic_pointer_cast<HttpResponse>(up->response());

    rsp.init(http_rsp->status_code, &http_rsp->headers, http_rsp->content_length, http_rsp->chunked);
    sp_info("status: %d, %lu, %d.", http_rsp->status_code, http_rsp->headers.size(), http_rsp->content_length);

    char   buf[1024];
    int    len  = sizeof(buf);
    size_t nr   = 0;
    rsp.set_send_timeout(10 * 1000 * 1000);
    rsp.set_recv_timeout(10 * 1000 * 1000);

    while ((ret = up->read(buf, len, nr)) == SUCCESS) {
        ret = rsp.write(buf, nr);

        if (ret != SUCCESS) {
            sp_error("Failed write url protocol %d.", ret);
            return ret;
        }
    }

    sp_trace("Final response code:%d, ret:%d, eof:%d", http_rsp->status_code, ret, ret == ERROR_URL_PROTOCOL_EOF);

    return (ret == ERROR_URL_PROTOCOL_EOF || ret == SUCCESS) ? SPS_HTTP_PHASE_SUCCESS_NO_CONTINUE : ret;
}

const char * HttpProxyPhaseHandler::get_name() {
    return "http-phase-proxy";
}

error_t Http404PhaseHandler::handler(HttpPhCtx& ctx) {
    auto socket = ctx.socket;
    PHttpResponseSocket http_socket = std::make_shared<HttpResponseSocket>(socket,
            socket->get_cip(), socket->get_port());

    if (!http_socket) {
        sp_error("Fatal not http socket type(socket):%s", typeid(ctx.socket.get()).name());
        return ERROR_SOCKET_CLOSED;
    }

    http_socket->init(404, nullptr, 0, false);

    auto ret =  http_socket->write_header();

    sp_trace("Response http 404 %d", ret);
    return SPS_HTTP_PHASE_SUCCESS_NO_CONTINUE;
}

const char* Http404PhaseHandler::get_name() {
    return "http-404-handler";
}

error_t HttpPhaseHandler::handler(HttpPhCtx& ctx) {
    error_t ret = SUCCESS;

    auto& filters = refs();

    if (filters.empty()) {
        return SingleInstance<Http404PhaseHandler>::get_instance().handler(ctx);
    }

    for (auto& f : filters) {
        ret = f->handler(ctx);
        if (ret == SPS_HTTP_PHASE_SUCCESS_NO_CONTINUE) {
            sp_trace("Success %s handler", f->get_name());

            return ret;
        } else if (ret == SPS_HTTP_PHASE_CONTINUE) {
            continue;
        } else {
            sp_error("Failed %s handler ret:%d", f->get_name(), ret);
            return ret;
        }
    }

    return ret;
}

HttpPhaseHandler::HttpPhaseHandler() {

}

}

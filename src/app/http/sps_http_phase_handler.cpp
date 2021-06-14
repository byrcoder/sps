#include <app/http/sps_http_phase_handler.hpp>
#include <app/http/sps_http_socket.hpp>
#include <log/sps_log.hpp>
#include <app/url/sps_url_protocol.hpp>
#include <memory>

namespace sps {

HostPhaseCtx::HostPhaseCtx(PRequestUrl r, PSocket s) {
    req      = std::move(r);
    socket   = std::move(s);
}

HttpParsePhaseHandler::HttpParsePhaseHandler() : IPhaseHandler("http-parser-handler") {
}

error_t HttpParsePhaseHandler::handler(HostPhaseCtx &ctx) {
    auto http_parser = std::make_shared<HttpParser>();
    error_t ret = SUCCESS;

    sp_debug("Request Http Parse");

    if ((ret = http_parser->parse_header(ctx.socket, HttpType::REQUEST)) <= SUCCESS) {
        sp_error("failed get http request ret:%d", ret);
        return ret;
    }

    ctx.req = http_parser->get_request();
    sp_trace("request info %s, %s", ctx.req->host.c_str(), ctx.req->url.c_str());

    return SPS_HTTP_PHASE_CONTINUE;
}

HttpProxyPhaseHandler::HttpProxyPhaseHandler() : IPhaseHandler("http-router-handler") {
}

error_t HttpProxyPhaseHandler::handler(HostPhaseCtx &ctx) {
    auto& host_ctx       = ctx.host;
    auto  host_conf     = std::static_pointer_cast<HostConfCtx>(host_ctx->conf);
    auto  proxy_req     = std::make_shared<RequestUrl>(*ctx.req);
    auto  n             = host_conf->pass_proxy.find(':');
    auto& protocols     = SingleInstance<UrlProtocol>::get_instance();
    auto  ret           = SUCCESS;

    if (n != std::string::npos) {
        proxy_req->ip   = host_conf->pass_proxy.substr(0, n);
        proxy_req->port = atoi(host_conf->pass_proxy.substr(n+1).c_str());
    } else {
        proxy_req->ip   = host_conf->pass_proxy;
        proxy_req->port = 80;
    }

    auto  url_protocol = protocols.create(proxy_req);

    if (!url_protocol) {
        sp_error("not found protocol %s for proxy", proxy_req->schema.c_str());
        return ERROR_UPSTREAM_NOT_FOUND;
    }

    if ((ret = url_protocol->open(proxy_req)) != SUCCESS) {
        sp_error("Failed open url protocol %s. %s, ret:%ld.",proxy_req->url.c_str(),
                host_conf->pass_proxy.c_str(), ret);
        return ret;
    }

    auto http_rsp = std::dynamic_pointer_cast<HttpResponse>(url_protocol->response());
    HttpResponseSocket rsp(ctx.socket, ctx.ip, ctx.port);

    rsp.init(http_rsp->status_code, &http_rsp->headers,
             http_rsp->content_length, http_rsp->chunked);

    if ((ret = rsp.write_header()) != SUCCESS) {
        sp_error("write head status: %d, %lu, %d, %ld.", http_rsp->status_code,
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
            sp_error("Failed write url protocol %ld.", ret);
            return ret;
        }
    }

    sp_trace("final response code:%d, ret:%ld, eof:%d", http_rsp->status_code,
              ret, ret == ERROR_HTTP_RES_EOF);

    return (ret == ERROR_HTTP_RES_EOF || ret == SUCCESS) ? SPS_HTTP_PHASE_SUCCESS_NO_CONTINUE : ret;
}

Http404PhaseHandler::Http404PhaseHandler() : IPhaseHandler("http-404-handler") {

}

error_t Http404PhaseHandler::handler(HostPhaseCtx& ctx) {
    auto socket = ctx.socket;
    PHttpResponseSocket http_socket = std::make_shared<HttpResponseSocket>(socket,
            socket->get_cip(), socket->get_port());

    if (!http_socket) {
        sp_error("Fatal not http socket type(socket):%s", typeid(ctx.socket.get()).name());
        return ERROR_HTTP_SOCKET_CREATE;
    }

    http_socket->init(404, nullptr, 0, false);

    auto ret =  http_socket->write_header();

    sp_trace("Response http 404 %d", ret);
    return SPS_HTTP_PHASE_SUCCESS_NO_CONTINUE;
}

error_t HttpPhaseHandler::handler(HostPhaseCtx& ctx) {
    error_t ret = SUCCESS;

    auto& filters = refs();

    if (filters.empty()) {
        return SingleInstance<Http404PhaseHandler>::get_instance().handler(ctx);
    }

    for (auto& f : filters) {
        ret = f->handler(ctx);
        if (ret == SPS_HTTP_PHASE_SUCCESS_NO_CONTINUE) {
            sp_debug("success %s handler", f->get_name());
            return SUCCESS;
        } else if (ret == SPS_HTTP_PHASE_CONTINUE) {
            continue;
        } else {
            sp_error("failed handler ret:%d", ret);
            return ret;
        }
    }

    return ret;
}

HttpPhaseHandler::HttpPhaseHandler() {

}

}

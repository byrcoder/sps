#include <app/http/phase_handler.hpp>
#include <app/http/socket.hpp>
#include <log/logger.hpp>

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

    return SUCCESS;
}

const char * HttpParsePhaseHandler::get_name() {
    return "http-parser-handler";
}

Http404PhaseHandler& Http404PhaseHandler::get_instance() {
    static Http404PhaseHandler filter404;
    return filter404;
}

error_t Http404PhaseHandler::handler(HttpPhCtx& ctx) {
    auto socket = ctx.socket;
    PHttpResponseSocket http_socket = std::dynamic_pointer_cast<HttpResponseSocket>(socket);

    if (!http_socket) {
        sp_error("Fatal not http socket type(socket):%s", typeid(ctx.socket.get()).name());
        return ERROR_SOCKET_CLOSED;
    }

    http_socket->init(404, nullptr, 0, false);

    auto ret =  http_socket->write_header();

    sp_trace("Response http 404 %d", ret);
    return ret;
}

const char* Http404PhaseHandler::get_name() {
    return "http-404-handler";
}

HttpPhaseHandler& HttpPhaseHandler::get_instance() {
    static HttpPhaseHandler filter;
    return filter;
}

error_t HttpPhaseHandler::handler(HttpPhCtx& ctx) {
    error_t ret = SUCCESS;

    if (filters.empty()) {
        return Http404PhaseHandler::get_instance().handler(ctx);
    }

    for (auto& f : filters) {
        if ((ret = f->handler(ctx)) != SUCCESS) {
            sp_error("Failed %s handler ret:%d", f->get_name(), ret);
            return ret;
        }
        sp_trace("Success %s handler", f->get_name());
    }

    return ret;
}

HttpPhaseHandler::HttpPhaseHandler() {
    filters.push_back(std::make_shared<HttpParsePhaseHandler>());
    filters.push_back(std::make_shared<Http404PhaseHandler>());
}

}

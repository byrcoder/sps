#include <app/http/phase_handler.hpp>
#include <app/http/socket.hpp>
#include <log/logger.hpp>

namespace sps {

HttpPhCtx::HttpPhCtx(PRequestUrl r, PSocket s) {
    req = std::move(r);
    s   = std::move(s);
}

Http404PhaseHandler& Http404PhaseHandler::get_instance() {
    static Http404PhaseHandler filter404;
    return filter404;
}

error_t Http404PhaseHandler::handler(HttpPhCtx& ctx) {
    PHttpResponseSocket http_socket = std::dynamic_pointer_cast<HttpResponseSocket>(ctx.socket);

    if (!http_socket) {
        sp_error("Fatal not http socket");
        return ERROR_SOCKET_CLOSED;
    }

    http_socket->init(404, nullptr, 0, false);

    return http_socket->write_header();
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
            return ret;
        }
    }

    return ret;
}

}

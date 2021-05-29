#include <http/filter.hpp>
#include <log/logger.hpp>

namespace sps {

Http404Filter& Http404Filter::get_instance() {
    static Http404Filter filter404;
    return filter404;
}

error_t Http404Filter::filter(PHttpRequest req, PClientSocket socket) {
    PHttpClientSocket http_socket = std::dynamic_pointer_cast<HttpClientSocket>(socket);

    if (!http_socket) {
        sp_error("Fatal not http socket");
        return ERROR_SOCKET_CLOSED;
    }

    http_socket->init(404, nullptr, 0, false);

    return http_socket->write_header();
}

HttpFilterLink& HttpFilterLink::get_instance() {
    static HttpFilterLink filter;
    return filter;
}

error_t HttpFilterLink::filter(PHttpRequest req, PClientSocket socket) {
    error_t ret = SUCCESS;

    if (filters.empty()) {
        return Http404Filter::get_instance().filter(req, socket);
    }

    for (auto& f : filters) {
        if ((ret = f->filter(req, socket)) != SUCCESS) {
            return ret;
        }
    }

    return ret;
}

}

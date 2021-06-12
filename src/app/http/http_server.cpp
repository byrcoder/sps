#include <app/http/http_server.hpp>

#include <app/http/http_phase_handler.hpp>
#include <app/http/http_socket.hpp>

#include <log/log_logger.hpp>

namespace sps {

HttpSocketHandler::HttpSocketHandler(PSocket io, PHttpPhaseHandler& handler) :
    ISocketHandler(std::move(io)), hd(handler) {

}

error_t HttpSocketHandler::handler() {
    HttpPhCtx ctx(nullptr, io);
    do {
        error_t ret = SUCCESS;

        if ((ret = hd->handler(ctx)) != SUCCESS) {
            return ret;
        }
        sp_trace("Success handler ret %d", ret);
    } while(true);

    return SUCCESS;
}

HttpHandlerFactory::HttpHandlerFactory(PHttpPhaseHandler hd) {
    handler = std::move(hd);
}

PISocketHandler HttpHandlerFactory::create(PSocket io) {
    return std::make_shared<HttpSocketHandler>(io, handler);
}

HttpServer::HttpServer(PHttpPhaseHandler handler, Transport transport) :
    Server(std::make_shared<HttpHandlerFactory>(std::move(handler)),
        transport) {
}

}

#include <app/http/http_server.hpp>

#include <app/http/http_phase_handler.hpp>
#include <app/http/http_parser.hpp>
#include <app/http/http_socket.hpp>

#include <log/log_logger.hpp>

namespace sps {

HttpSocketHandler::HttpSocketHandler(PSocket io) : ISocketHandler(std::move(io)) { }

error_t HttpSocketHandler::handler() {
    HttpPhCtx ctx(nullptr, io);
    do {
        error_t ret = SUCCESS;

        if ((ret = SingleInstance<HttpPhaseHandler>::get_instance().handler(ctx)) != SUCCESS) {
            return ret;
        }
        sp_trace("Success handler ret %d", ret);
    } while(true);

    return SUCCESS;
}

PISocketHandler HttpHandlerFactory::create(PSocket io) {
    // PSocket http_socket = std::make_shared<HttpResponseSocket>(io, io->get_cip(), io->get_port());
    return std::make_shared<HttpSocketHandler>(io);
}

HttpServer::HttpServer(Transport transport) : Server(std::make_shared<HttpHandlerFactory>(), transport) {
}

}

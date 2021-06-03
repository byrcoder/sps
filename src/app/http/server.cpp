#include <app/http/server.hpp>

#include <app/http/phase_handler.hpp>
#include <app/http/parser.hpp>
#include <app/http/socket.hpp>

#include <log/logger.hpp>

namespace sps {

HttpSocketHandler::HttpSocketHandler(PSocket io) : ISocketHandler(std::move(io)) { }

error_t HttpSocketHandler::handler() {
    do {
        auto ret = SUCCESS;
        HttpPhCtx ctx(nullptr, io);
        if ((ret = SingleInstance<HttpPhaseHandler>::get_instance().handler(ctx)) != SUCCESS) {
            return ret;
        }
    } while(true);

    return SUCCESS;
}

PISocketHandler HttpHandlerFactory::create(PSocket io) {
    PSocket http_socket = std::make_shared<HttpResponseSocket>(io, io->get_cip(), io->get_port());
    return std::make_shared<HttpSocketHandler>(http_socket);
}

HttpServer::HttpServer(Transport transport) : Server(std::make_shared<HttpHandlerFactory>(), transport) {
}

}

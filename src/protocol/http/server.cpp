#include <protocol/http/server.hpp>

#include <protocol/http/filter.hpp>
#include <protocol/http/parser.hpp>
#include <protocol/http/socket.hpp>

#include <log/logger.hpp>

namespace sps {

HttpSocketHandler::HttpSocketHandler(PSocket io) : ISocketHandler(std::move(io)) { }

error_t HttpSocketHandler::handler() {
    do {
        auto http_parser = std::make_shared<HttpParser>();
        error_t ret = SUCCESS;

        sp_trace("Request Http Parse");

        if ((ret = http_parser->parse_header(io, HttpType::REQUEST)) <= SUCCESS) {
            return ret;
        }

        auto req = http_parser->get_request();
        sp_trace("Request %s, %s, %s", req->host.c_str(), req->url.c_str(), req->params.c_str());

        http_parser.reset(); // release memory

        if ((ret = HttpFilterLink::get_instance().filter(req, io)) != SUCCESS) {
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

#include <server/http_server.hpp>

#include <protocol/http/filter.hpp>
#include <protocol/http/parser.hpp>
#include <protocol/http/socket.hpp>

#include <log/logger.hpp>

namespace sps {

HttpHandler::HttpHandler(PSocket io) : ISocketHandler(io) {

}

HttpHandler::~HttpHandler() {

}

error_t HttpHandler::handler() {
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
    return std::make_shared<HttpHandler>(http_socket);
}

HttpServer::HttpServer(Transport transport) : IServer(std::make_shared<HttpHandlerFactory>()) {
    this->transport = transport;
}

int HttpServer::listen(const std::string &ip, int port) {
    ssocket = ServerSocketFactory::get_instance().create_ss(Transport::TCP);

    if (!ssocket) return ERROR_ST_OPEN_SOCKET;

    return ssocket->listen(ip, port);
}

PSocket HttpServer::do_accept() {
    return ssocket->accept();
}

}
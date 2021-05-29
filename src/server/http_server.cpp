#include <server/http_server.hpp>
#include <http/parser.hpp>
#include <http/filter.hpp>
#include <log/logger.hpp>

namespace sps {

HttpHandler::HttpHandler(PClientSocket io) : ISocketHandler(io) {

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

PISocketHandler HttpHandlerFactory::create(PClientSocket io) {
    PHttpClientSocket http_socket = std::make_shared<HttpClientSocket>(io, io->get_cip(), io->get_port());
    return std::make_shared<HttpHandler>(http_socket);
}

HttpServer::HttpServer(Transport transport) : IServer(std::make_shared<HttpHandlerFactory>()) {
    this->transport = transport;
}

int HttpServer::listen(const std::string &ip, int port) {
    ssocket = IServerSocketFactory::get_instance().create_ss(Transport::TCP);

    if (!ssocket) return ERROR_ST_OPEN_SOCKET;

    return ssocket->listen(ip, port);
}

PClientSocket HttpServer::do_accept() {
    return ssocket->accept();
}

}
#include <server/http_server.hpp>
#include <http/parser.hpp>
#include <log/logger.hpp>

namespace sps {

HttpHandler::HttpHandler(PClientSocket io) : ISocketHandler(io) {

}

HttpHandler::~HttpHandler() {
}

error_t HttpHandler::handler() {
    HttpParser http_parser;
    error_t ret = SUCCESS;

    sp_trace("Request Http Parse");

    if ((ret = http_parser.parse_header(io, HttpType::REQUEST)) != SUCCESS) {
        return ret;
    }

    auto ctx = http_parser.get_ctx();
    sp_trace("Request %s, %s, %s", ctx->host.c_str(), ctx->url.c_str(), ctx->params.c_str());

    return SUCCESS;
}

PISocketHandler HttpHandlerFactory::create(PClientSocket io) {
    return std::make_shared<HttpHandler>(io);
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
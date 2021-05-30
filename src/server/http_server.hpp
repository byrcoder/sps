#ifndef SPS_SERVER_HTTP_SERVER_HPP
#define SPS_SERVER_HTTP_SERVER_HPP

#include <net/socket.hpp>

#include <server/server.hpp>
#include <protocol/http/filter.hpp>

namespace sps {

class HttpHandler : public ISocketHandler {
 public:
    HttpHandler(PSocket io);
    ~HttpHandler();

 public:
    error_t handler();
};

class HttpHandlerFactory : public ISocketHandlerFactory {
 public:
    PISocketHandler create(PSocket io);
};

class HttpServer : public IServer {
 public:
    HttpServer(Transport transport = Transport::TCP);

 public:
    int listen(const std::string& ip, int port) override;
    virtual PSocket do_accept() override;

 private:
    Transport transport;
    PIServerSocket ssocket;
};

}

#endif  // SPS_SERVER_HTTP_SERVER_HPP

#ifndef SPS_PROTOCOL_HTTP_SERVER_HPP
#define SPS_PROTOCOL_HTTP_SERVER_HPP

#include <app/server/server.hpp>
#include <net/socket.hpp>

namespace sps {

class HttpSocketHandler : public ISocketHandler {
 public:
    explicit HttpSocketHandler(PSocket io);

 public:
    error_t handler() override ;
};

class HttpHandlerFactory : public ISocketHandlerFactory {
 public:
    PISocketHandler create(PSocket io) override;
};

class HttpServer : public Server {
 public:
    explicit HttpServer(Transport transport = Transport::TCP);
};

}

#endif  // SPS_PROTOCOL_HTTP_SERVER_HPP

#ifndef SPS_HTTP_SERVER_HPP
#define SPS_HTTP_SERVER_HPP

#include <app/server/sps_server.hpp>
#include <net/sps_net_socket.hpp>
#include "sps_http_phase_handler.hpp"

namespace sps {

class HttpSocketHandler : public ISocketHandler {
 public:
    explicit HttpSocketHandler(PSocket io, PHttpPhaseHandler& handler);

 public:
    error_t handler() override;

 public:
    PHttpPhaseHandler& hd;
};

class HttpHandlerFactory : public ISocketHandlerFactory {
 public:
    explicit HttpHandlerFactory(PHttpPhaseHandler hd);
    PISocketHandler create(PSocket io) override;

 private:
    PHttpPhaseHandler handler;
};

class HttpServer : public Server {
 public:
    explicit HttpServer(PHttpPhaseHandler handler, Transport transport = Transport::TCP);
};

}

#endif  // SPS_HTTP_SERVER_HPP

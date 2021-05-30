#ifndef SPS_SERVER_SERVER_HPP
#define SPS_SERVER_SERVER_HPP

#include <memory>
#include <set>

#include <co/co.hpp>
#include <net/socket.hpp>

namespace sps {

class ISocketHandler;
typedef std::shared_ptr<ISocketHandler> PISocketHandler;

class SocketManager {
 public:
    static SocketManager& get_manage();

 public:
    int add(PISocketHandler h);
    int remove(PISocketHandler h);

 private:
    std::set<PISocketHandler> sockets;
};

class ISocketHandler: public ICoHandler, public std::enable_shared_from_this<ISocketHandler> {
 public:
    explicit ISocketHandler(PSocket io);

 public:
    void on_stop() override;

 protected:
    PSocket io;
};
typedef std::shared_ptr<ISocketHandler> PISocketHandler;

class ISocketHandlerFactory {
 public:
    ~ISocketHandlerFactory() = default;

 public:
    virtual PISocketHandler create(std::shared_ptr<Socket> io) = 0;
};
typedef std::shared_ptr<ISocketHandlerFactory> PISocketHandlerFactory;

/**
 * 一个server必须实现listener, handler factory 和 handler
 */
class Server : public ICoHandler {
 public:
    explicit Server(PISocketHandlerFactory factory, Transport transport);
    ~Server() override = default;

 public:
    int listen(std::string ip, int port, bool reuse_port = true, int backlog = 1024);

 public:
    virtual error_t accept();
    error_t handler() override;

 protected:
    PISocketHandlerFactory factory;
    PIServerSocket         server_socket;
    Transport              tran;
};

}

#endif  // SPS_SERVER_SERVER_HPP

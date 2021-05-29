#ifndef SPS_SERVER_HPP
#define SPS_SERVER_HPP

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
    ISocketHandler(PClientSocket io);
    virtual ~ISocketHandler() = default;

 public:
    void on_stop();

 protected:
    PClientSocket io;
};
typedef std::shared_ptr<ISocketHandler> PISocketHandler;

class ISocketHandlerFactory {
 public:
    ~ISocketHandlerFactory() = default;

 public:
    virtual PISocketHandler create(std::shared_ptr<ClientSocket> io) = 0;
};
typedef std::shared_ptr<ISocketHandlerFactory> PISocketHandlerFactory;

class IServer : public ICoHandler {
 public:
    explicit IServer(PISocketHandlerFactory factory);
    virtual ~IServer() = default;

 public:
    virtual int listen(const std::string& ip, int port) = 0;
    virtual PClientSocket do_accept() = 0;

 public:
    virtual error_t accept();

    error_t handler() override;

 private:
    PISocketHandlerFactory factory;
};

}

#endif //SPS_SERVER_HPP

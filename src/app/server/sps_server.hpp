/*****************************************************************************
MIT License
Copyright (c) 2021 byrcoder

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*****************************************************************************/

#ifndef SPS_SERVER_HPP
#define SPS_SERVER_HPP

#include <memory>
#include <set>

#include <co/sps_co.hpp>
#include <net/sps_net_socket.hpp>

namespace sps {

class ISocketHandler;
typedef std::shared_ptr<ISocketHandler> PISocketHandler;

class SocketManager : public Registers<SocketManager, PISocketHandler> {

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
typedef std::shared_ptr<Server> PServer;

}

#endif  // SPS_SERVER_HPP

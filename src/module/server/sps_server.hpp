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

#include <sps_co.hpp>
#include <sps_io_socket.hpp>

namespace sps {

class IConnectionHandler;
typedef std::shared_ptr<IConnectionHandler> PIConnectionHandler;

/**
 * the assemble for all connections
 */
class ConnectionManager : public Registers<ConnectionManager, PIConnectionHandler> {

};

/**
 * this work for every connection
 */
class IConnectionHandler: public ICoHandler, public std::enable_shared_from_this<IConnectionHandler> {
 public:
    explicit IConnectionHandler(PSocket io);

 public:
    void on_stop() override;

 protected:
    PSocket io;
};
typedef std::shared_ptr<IConnectionHandler> PIConnectionHandler;

/**
 * every kind connection-handler has a factory
 */
class IConnectionHandlerFactory {
 public:
    ~IConnectionHandlerFactory() = default;

 public:
    virtual PIConnectionHandler create(std::shared_ptr<Socket> io) = 0;
};
typedef std::shared_ptr<IConnectionHandlerFactory> PIConnectionHandlerFactory;

/**
 * 一个server必须实现listener, handler factory 和 handler
 */
class Server : public ICoHandler {
 public:
    explicit Server() = default;
    ~Server() override = default;

 public:
    error_t init(PIConnectionHandlerFactory factory, Transport transport);
    error_t listen(std::string ip, int port, bool reuse_port = true, int backlog = 1024);

 public:
    virtual error_t accept();
    error_t handler() override;

 protected:
    PIConnectionHandlerFactory factory;
    PIServerSocket             server_socket;
    Transport                  tran;
};
typedef std::shared_ptr<Server> PServer;

}

#endif  // SPS_SERVER_HPP

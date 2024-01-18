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
#include <string>
#include <utility>

#include <sps_sys_co.hpp>
#include <sps_io_socket.hpp>
#include <sps_sys_st_io_ssl.hpp>

namespace sps {

class IConnHandler;
typedef std::shared_ptr<IConnHandler> PIConnHandler;

/**
 * the assemble for all connections
 */
class ConnManager : public Registers<ConnManager, PIConnHandler> {
};

/**
 * this work for every connection
 */
class IConnHandler: public ICoHandler,
                    public std::enable_shared_from_this<IConnHandler> {
 public:
    explicit IConnHandler(PSocket io);

 public:
    void on_stop() override;

 public:
    PSocket io;
};
typedef std::shared_ptr<IConnHandler> PIConnHandler;

/**
 * every kind connection-handler has a factory
 */
class IConnHandlerFactory {
 public:
    ~IConnHandlerFactory() = default;

 public:
    virtual PIConnHandler create(std::shared_ptr<Socket> io) = 0;
};
typedef std::shared_ptr<IConnHandlerFactory> PIConnHandlerFactory;

/**
 * every server must impl listener, handler factory 和 handler
 */
class Server : public ICoHandler {
 public:
    Server() = default;
    ~Server() override = default;

 public:
    error_t init(PIConnHandlerFactory factory,
                 utime_t send_timeout, utime_t  rcv_timeout);
    void stop();

#ifdef OPENSSL_ENABLED
    void    init_ssl(const std::string& crt_file, const std::string& key_file,
            PISSLCertSearch searcher = nullptr);
#endif

    error_t listen(std::string ip, int port, bool reuse_port = true,
                   int backlog = 1024, Transport t = DEFAULT);

 public:
    virtual error_t accept();
    error_t handler() override;

 protected:
    bool                       running = false;
    PIConnHandlerFactory       factory;
    PIServerSocket             server_socket;
    utime_t                    recv_timeout = -1;
    utime_t                    send_timeout = -1;
    Transport                  tran;
    std::string                listen_ip;
    int                        listen_port;
};
typedef std::shared_ptr<Server> PServer;

}  // namespace sps

#endif  // SPS_SERVER_HPP

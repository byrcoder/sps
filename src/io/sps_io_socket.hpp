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

#ifndef SPS_IO_SOCKET_HPP
#define SPS_IO_SOCKET_HPP

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>

#include <climits>
#include <list>
#include <memory>
#include <string>
#include <utility>

#include <sps_io.hpp>

namespace sps {

enum Transport {
    TCP,
#ifdef SRT_ENABLED
    SRT,
#endif
    QUIC,
    FILE,
#ifdef OPENSSL_ENABLED
    HTTPS,
#endif
    DEFAULT = 0xFF
};

bool addr2string(const struct sockaddr_in* addr, std::string& ip, int& port);

const char* get_transport_name(Transport t);

// 适配器io， 统一的类型
class Socket : public IReaderWriter {
 public:
    Socket();
    Socket(PIReaderWriter rw, const std::string& peer_ip, int peer_port,
           const std::string& self_ip = "", int self_port = -1);

 public:
    void init(PIReaderWriter rw, const std::string& ip, int p, const std::string& sip, int sp);

 public:
    error_t read_fully(void* buf, size_t size, ssize_t* nread) override;
    error_t read(void* buf, size_t size, size_t& nread) override;
    void   set_recv_timeout(utime_t tm) override;
    utime_t get_recv_timeout() override;
    bool seekable() override;

 public:
    error_t write(void* buf, size_t size) override;
    void    set_send_timeout(utime_t tm) override;
    utime_t get_send_timeout() override;

 public:
    PIReaderWriter get_io();
    std::string    get_peer_ip() const;
    int            get_peer_port() const;
    std::string    get_self_ip()    const;
    int            get_self_port()  const;

 protected:
    PIReaderWriter io;
    std::string peer_ip;
    int         peer_port;
    std::string self_ip;
    int         self_port;
};

typedef std::shared_ptr<Socket> PSocket;

class ClientSocketFactory : public Single<ClientSocketFactory> {
 public:
    PSocket create_ss(Transport transport,
                      const std::string& ip,
                      int port, utime_t tm);
};

/**
* tcp serversocket只能是监听数据，udp的可以发送数据，防止乱用，默认抛出错误
*/
class IServerSocket {
 public:
    virtual ~IServerSocket() = default;

 public:
    virtual error_t listen(std::string ip, int port,
                           bool reuse_port = true,
                           int backlog = 1024) = 0;
    virtual PSocket accept()  = 0;
};

typedef std::shared_ptr<IServerSocket> PIServerSocket;

class ServerSocketFactory : public Single<ServerSocketFactory> {
 public:
    PIServerSocket create_ss(Transport transport);
};

}  // namespace sps

#endif  // SPS_IO_SOCKET_HPP

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

#ifndef SPS_ST_IO_UDP_HPP
#define SPS_ST_IO_UDP_HPP

#include <sps_io_socket.hpp>
#include <sps_st_io_tcp.hpp>
#include <sps_co.hpp>
#include <sps_sync.hpp>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>

namespace sps {

class UdpFd;
typedef std::shared_ptr<UdpFd> PUdpFd;
class UdpFd {
 public:
    static error_t create_fd(PUdpFd& fd);
    static error_t create_fd(int bind_port, PUdpFd& fd,
                             bool reuse_addr = false,
                             bool reuse_port = false);

 public:
    explicit UdpFd(st_netfd_t udp_fd);
    ~UdpFd();

    st_netfd_t get_fd();

 private:
    st_netfd_t udp_fd;
};

class StUdpClientSocket : public IReaderWriter {
 public:
    explicit StUdpClientSocket(const std::string& peer_ip, int peer_port, PUdpFd fd);

 public:
    // reader
    void    set_recv_timeout(utime_t tm) override;
    utime_t get_recv_timeout() override;

    error_t connect();
    error_t read_fully(void* buf, size_t size, ssize_t* nread) override;
    error_t read(void* buf, size_t size, size_t& nread) override;

 public:
    // udp addresss
    error_t read(void* buf, size_t size, size_t& nread, std::string& peer_ip, int& peer_port);

 public:
    // writer
    void set_send_timeout(utime_t tm) override;
    utime_t get_send_timeout() override;
    error_t write(void* buf, size_t size) override;

 private:
    utime_t rtm;
    utime_t stm;
    // The recv/send data in bytes
    int64_t rbytes;
    int64_t sbytes;
    // The underlayer st fd.
    PUdpFd fd;

    // peer address
    std::string peer_ip;
    int         peer_port;
    sockaddr_in peer_addr;
    bool        connected;
};

typedef std::shared_ptr<StUdpClientSocket> PStUdpClientSocket;

class UdpBuffer {
 public:
    UdpBuffer(uint8_t* buf, size_t len);
    ~UdpBuffer();

 private:
    UdpBuffer(const UdpBuffer& buffer);

 public:
    uint8_t* buffer();
    size_t   size();

 private:
    uint8_t* buf;
    size_t   len;
};

typedef std::shared_ptr<UdpBuffer> PUdpBuffer;

/**
 * udp session wrap
 * read from udp server
 * sent by udp fd
 */
class StUdpSessionClientSocket : public StUdpClientSocket {
 public:
    StUdpSessionClientSocket(const std::string& cip, int cport, PUdpFd fd);
    ~StUdpSessionClientSocket() override;

 public:
    error_t read(void* buf, size_t size, size_t& nread) override;

 public:
    error_t push(uint8_t* buf, size_t len);

 private:
    std::list<PUdpBuffer> buffer_lists;
    PCondition cond;
    int buffer_max_list_size;
    int buffer_list_size;
};
typedef std::shared_ptr<StUdpSessionClientSocket> PStUdpSessionClientSocket;

class IUdpSocketManager;
class UdpSessionSocket : public Socket {
 public:
    UdpSessionSocket(IUdpSocketManager* manger);
    UdpSessionSocket(IUdpSocketManager* manager, PIReaderWriter rw, const std::string& ip, int p);
    ~UdpSessionSocket() override;

 private:
    IUdpSocketManager* manager;
};

class IUdpSocketManager {
 public:
    virtual void on_destroy(UdpSessionSocket* session) = 0;
};

class StUdpServerSocket : public IServerSocket, public IUdpSocketManager, public ICoHandler,
        public std::enable_shared_from_this<StUdpServerSocket> {
 public:
    StUdpServerSocket() = default;
    ~StUdpServerSocket();

 public:
    error_t listen(std::string sip, int sport, bool reuse_sport, int back_log) override;
    PSocket accept() override;
    error_t handler() override;

 public:
    void on_destroy(UdpSessionSocket* session) override;

 private:
    std::string generate_key(const std::string& cip, int cport);
    PStUdpSessionClientSocket search_or_create(std::string& cip, int cport, bool &is_new);
    void remove(const std::string& cip, int cport);

 private:
    uint8_t* buffer = nullptr;
    int      buffer_size = 4096;
    std::string ip;
    int         port;
    bool        reuse_port;
    PUdpFd      server_fd;
    PStUdpClientSocket server_socket;

 private:
    std::map<std::string, std::weak_ptr<StUdpSessionClientSocket>> sessions;
    std::list<std::shared_ptr<UdpSessionSocket>> new_clients;
    PCondition new_cond;
};

}  // namespace sps

#endif  // SPS_ST_IO_UDP_HPP

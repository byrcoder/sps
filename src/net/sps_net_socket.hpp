#ifndef SPS_NET_SOCKET_HPP
#define SPS_NET_SOCKET_HPP

#include <climits>
#include <list>
#include <string>

#include <net/sps_net_io.hpp>

namespace sps {

enum Transport {
    TCP = 1,
#ifndef SRT_DISABLED
    SRT = 2,
#endif
    QUIC = 3,


    DEFAULT = 0xFF
};

// 适配器io， 统一的类型
class Socket : public IReaderWriter {
 public:
    Socket()  {
        init(nullptr, "", -1);
    }

    Socket(PIReaderWriter rw, const std::string& ip, int p) {
        init(std::move(rw), ip, p);
    }

 public:
    void init(PIReaderWriter rw, const std::string& ip, int p) {
        this->io    = std::move(rw);
        this->cip   = ip;
        this->port  = p;
    }

 public:
    error_t read_fully(void* buf, size_t size, ssize_t* nread) override { return io->read_fully(buf, size, nread); }
    error_t read(void* buf, size_t size, size_t& nread) override        { return io->read(buf, size, nread);    }

    void    set_recv_timeout(utime_t tm) override {    io->set_send_timeout(tm);      }
    utime_t get_recv_timeout() override           {    return io->get_recv_timeout(); }
    bool    seekable() override                   {    return io->seekable();         }

 public:
    error_t write(void* buf, size_t size) override { return io->write(buf, size);     }
    void    set_send_timeout(utime_t tm) override  { return io->set_send_timeout(tm); }
    utime_t get_send_timeout() override            { return io->get_send_timeout();   }

 public:
    PIReaderWriter get_io()              {  return io;   }
    std::string    get_cip()   const     {  return cip;  }
    int            get_port()   const    {  return port; }

 protected:
    PIReaderWriter io;
    std::string cip;
    int         port;
};

typedef std::shared_ptr<Socket> PSocket;

class ClientSocketFactory : public Single<ClientSocketFactory> {
 public:
    PSocket create_ss(Transport transport, const std::string& ip, int port, utime_t tm);
};

/**
* tcp serversocket只能是监听数据，udp的可以发送数据，防止乱用，默认抛出错误
*/
class IServerSocket {
 public:
    virtual ~IServerSocket() = default;

 public:
    virtual error_t listen(std::string ip, int port, bool reuse_port = true, int backlog = 1024) = 0;
    virtual PSocket accept()  = 0;
};

typedef std::shared_ptr<IServerSocket> PIServerSocket;

class ServerSocketFactory : public Single<ServerSocketFactory> {
 public:
    PIServerSocket create_ss(Transport transport);
};

}

#endif  // SPS_NET_SOCKET_HPP

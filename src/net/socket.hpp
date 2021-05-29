#include <climits>

#ifndef SPS_SOCKET_HPP
#define SPS_SOCKET_HPP

#include <string>

#include <net/io.hpp>
#include <list>

namespace sps {

enum Transport {
    TCP = 1,
#ifndef SRT_DISABLED
    SRT = 2,
#endif
    QUIC = 3,
};

// 搞一个适配器，封装socket
class ClientSocket : public IReaderWriter {
 public:
    ClientSocket(PIReaderWriter io, const std::string& ip, int port) {
        this->io    = std::move(io);
        this->cip   = ip;
        this->port  = port;
    }

 public:
    error_t read_fully(void* buf, size_t size, ssize_t* nread = nullptr) override { return io->read_fully(buf, size, nread); }
    error_t read(void* buf, size_t size, size_t& nread) override                  { return io->read(buf, size, nread);    }

    void    set_recv_timeout(utime_t tm) override {    io->set_send_timeout(tm);      }
    utime_t get_recv_timeout() override           {    return io->get_recv_timeout(); }
    bool    seekable() override                   {    return io->seekable();         }

 public:
    error_t write(void* buf, size_t size) override { return io->write(buf, size);     }
    void    set_send_timeout(utime_t tm) override  { return io->set_send_timeout(tm); }
    utime_t get_send_timeout() override            { return io->get_send_timeout();   }

 public:
    PIReaderWriter get_io()              { return io;    }
    std::string    get_cip()             { return cip;   }
    int            get_port()            {  return port; }

 protected:
    PIReaderWriter io;
    std::string cip;
    int         port;
};

typedef std::pair<std::string, std::string> Header ;
class HttpClientSocket : public ClientSocket {
 public:
    HttpClientSocket(PIReaderWriter io, const std::string& ip, int port);
 public:
    error_t init(int status_code, std::list<Header>* headers, int content_length, bool chunked);
    error_t write_header();
    error_t write(void* buf, size_t size) override;

 private:
    int  status_code    = 200;
    bool sent_header    = false;
    int  content_length = 0;
    bool chunked        = false;
    std::list<Header> headers;
};

typedef std::shared_ptr<HttpClientSocket> PHttpClientSocket;

typedef std::shared_ptr<ClientSocket> PClientSocket;

/**
* tcp serversocket只能是监听数据，udp的可以发送数据，防止乱用，默认抛出错误
*/
class IServerSocket : public IReaderWriter {
 public:
    virtual ~IServerSocket() = default;

 public:
    error_t read_fully(void* buf, size_t size, ssize_t* nread = nullptr) {
        throw "not impl";
    }

    error_t read(void* buf, size_t size, size_t& nread) {
        throw "not impl";
    }

    void set_recv_timeout(utime_t tm) {
        throw "not impl";
    }

    utime_t get_recv_timeout() {
        throw "not impl";
    }

 public:
    error_t write(void* buf, size_t size) {
        throw "not impl";
    }

    void set_send_timeout(utime_t tm) {
        throw "not impl";
    }

    utime_t get_send_timeout() {
        throw "not impl";
    }

 public:
    virtual error_t listen(std::string ip, int port, bool reuseport = true, int backlog = 1024) = 0;
    virtual PClientSocket accept()  = 0;
};

typedef std::shared_ptr<IServerSocket> PIServerSocket;

class IServerSocketFactory {
 public:
    static IServerSocketFactory& get_instance();

 public:
    PIServerSocket create_ss(Transport transport);
};


}

#endif  // SPS_SOCKET_HPP

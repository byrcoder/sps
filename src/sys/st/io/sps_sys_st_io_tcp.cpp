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

#include <sps_sys_st_io_tcp.hpp>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <sys/socket.h>

#include <memory>

#include <sps_io_socket.hpp>
#include <sps_log.hpp>

namespace sps {

int SockOpt::apply(st_netfd_t fd) {
    if (setsockopt(st_netfd_fileno(fd), level, optname, optval, optlen) < 0) {
        sp_error("SockOpt:::%d %d %d", st_netfd_fileno(fd), optname);
        return -1;
    }
    return 0;
}

std::string dns_resolve(const std::string& host) {
    if (inet_addr(host.c_str()) != INADDR_NONE) {
        return host;
    }

    hostent* answer = gethostbyname(host.c_str());
    if (answer == nullptr) {
        return "";
    }

    char ipv4[16];
    memset(ipv4, 0, sizeof(ipv4));
    if (0 < answer->h_length) {
        inet_ntop(AF_INET, answer->h_addr_list[0], ipv4, sizeof(ipv4));
    }

    return ipv4;
}

int st_tcp_fd(st_netfd_t fd) {
    return st_netfd_fileno(fd);
}

error_t st_tcp_create_fd(st_netfd_t& fd) {
    fd = nullptr;
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        return ERROR_SOCKET_CREATE;
    }
    fd = st_netfd_open_socket(sock);
    if (fd == nullptr) {
        ::close(sock);
        return ERROR_ST_OPEN_SOCKET;
    }
    return SUCCESS;
}

st_netfd_t st_tcp_open_fd(int fd) {
    return st_netfd_open_socket(fd);
}

error_t st_tcp_close(st_netfd_t& fd) {
    st_netfd_close(fd);
    fd = nullptr;
    return SUCCESS;
}

error_t st_tcp_listen(const std::string& server, int port,
                      int back_log, st_netfd_t& fd, bool reuse_port) {
    error_t ret = st_tcp_create_fd(fd);

    if (ret != SUCCESS) {
        return ret;
    }

    int       fileno  = st_netfd_fileno(fd);

    if ((ret = st_tcp_fd_reuseaddr(fileno)) != SUCCESS) {
        goto failed;
    }

    if (reuse_port) {
        if ((ret = st_tcp_fd_reuseport(fileno)) != SUCCESS) {
            goto failed;
        }
    }

    struct sockaddr_in addr;
    addr.sin_family        =   AF_INET;
    addr.sin_addr.s_addr   =   htonl(INADDR_ANY);
    addr.sin_port          =   htons(port);

    if (::bind(fileno, (struct sockaddr*) &addr, sizeof(addr)) == -1) {
        sp_error("bind addr %s:%d failed, fileno:%d",
                  server.c_str(), port, fileno);
        ret = ERROR_SOCKET_BIND;
        goto failed;
    }

    if (::listen(fileno, back_log) == -1) {
        ret = ERROR_SOCKET_LISTEN;
        goto failed;
    }

    return ret;

failed:
    st_tcp_close(fd);
    return ret;
}

error_t st_tcp_connect(const std::string& server, int port,
                       utime_t tm, st_netfd_t* fd, std::string* peer_ip) {
    error_t ret     = SUCCESS;
    *fd             = nullptr;
    st_netfd_t stfd = nullptr;
    sockaddr_in addr{};

    st_tcp_create_fd(stfd);
    if (stfd == nullptr) {
        ret = ERROR_ST_OPEN_SOCKET;
        return ret;
    }

    // connect to server.
    std::string ip = dns_resolve(server);
    if (ip.empty()) {
        ret = ERROR_SYSTEM_IP_INVALID;
        goto failed;
    }

    if (peer_ip) {
        *peer_ip = ip;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip.c_str());

    if (st_connect(stfd, (const struct sockaddr*) &addr, (int) sizeof(addr),
                   tm) == -1) {
        sp_error("failed tcp connect [%s] %s:%d, tm:%llu, ret:%d", server.c_str(),
                 ip.c_str(), port, tm, ret);
        ret = ERROR_ST_CONNECT;
        goto failed;
    }

    *fd = stfd;
    return ret;

failed:
    if (stfd) {
        st_tcp_close(stfd);
    }
    return ret;
}

st_netfd_t st_tcp_accept(st_netfd_t stfd, struct sockaddr *addr, int *addrlen,
                         utime_t timeout) {
    return st_accept(stfd, addr, addrlen, timeout);
}

error_t st_tcp_read(st_netfd_t stfd, void *buf, size_t nbyte, utime_t timeout) {
    return st_read(stfd, buf, nbyte, timeout);
}

error_t st_tcp_write(st_netfd_t stfd,  void *buf, size_t nbyte,
                     utime_t timeout) {
    return st_write(stfd, buf, nbyte, timeout);
}

error_t st_tcp_writev(st_netfd_t stfd, const iovec *iov,
                      int iov_size, utime_t timeout, size_t *nwrite) {
    ssize_t n = st_writev(stfd, iov, iov_size, timeout);
    if (n > 0) {
        *nwrite = n;
    }
    return n;
}

error_t st_tcp_fd_reuseaddr(int fd, int enable) {
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) == -1) {
        return ERROR_SOCKET_SETREUSE;
    }

    return SUCCESS;
}

error_t st_tcp_fd_reuseport(int fd) {
    int v = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &v, sizeof(int)) == -1) {
        return ERROR_SOCKET_SETREUSE;
    }
    return SUCCESS;
}

StTcpSocket::StTcpSocket(st_netfd_t stfd) {
    this->stfd = stfd;
    rtm = stm = ST_UTIME_NO_TIMEOUT;
    rbytes = sbytes = 0;
}

StTcpSocket::~StTcpSocket() {
    sp_debug("tcp socket close, fd:%d", st_tcp_fd(stfd));
    st_tcp_close(stfd);
}

// reader
void StTcpSocket::set_recv_timeout(utime_t tm) {
    rtm = tm;
}

utime_t StTcpSocket::get_recv_timeout() {
    return rtm;
}

error_t StTcpSocket::read_fully(void* buf, size_t size, ssize_t* nread) {
    error_t ret = SUCCESS;
    ssize_t n   = st_read_fully(stfd, buf, size, rtm);

    if (nread) {
        *nread = n;
    }

    if (n != (ssize_t)size) {
        // @see https://github.com/ossrs/srs/issues/200
        if (n < 0 && errno == ETIME) {
            return ERROR_SOCKET_TIMEOUT;
        }

        if (n >= 0) {
            errno = ECONNRESET;
        }

        return ERROR_SOCKET_READ_FULLY;
    }

    rbytes += n;
    return ret;
}

error_t StTcpSocket::read(void* buf, size_t size, size_t& nread) {
    if (size == 0) {
        return ERROR_IO_BUFFER_FULL;
    }

    error_t nb_read = st_read(stfd, buf, size, rtm);
    if (nb_read <= 0) {
        if (nb_read < 0 && errno == ETIME) {
            return ERROR_SOCKET_TIMEOUT;
        }

        if (nb_read == 0) {
            errno = ECONNRESET;
            return ERROR_SOCKET_CLOSED;
        }

        if (errno == EINTR) {
            return ERROR_SOCKET_EINTR;
        }
        return ERROR_SOCKET_READ;
    }

    rbytes += nb_read;
    nread   = nb_read;
    return SUCCESS;
}

void StTcpSocket::set_send_timeout(utime_t tm) {
    this->stm = tm;
}

utime_t StTcpSocket::get_send_timeout() {
    return stm;
}

error_t StTcpSocket::write(void* buf, size_t size) {
    error_t ret = SUCCESS;

    ssize_t nb_write = st_write(stfd, buf, size, stm);

    if (nb_write <= 0) {
        if (nb_write < 0 && errno == ETIME) {
            return ERROR_SOCKET_TIMEOUT;
        }
        return ERROR_SOCKET_WRITE;
    }

    sbytes += nb_write;

    return ret;
}

StTcpServerSocket::StTcpServerSocket() {
    port       = -1;
    server_fd  = nullptr;
    backlog    = 1024;
    reuse_port = false;
}

StTcpServerSocket::~StTcpServerSocket()  {
    if (server_fd) st_tcp_close(server_fd);
}

error_t StTcpServerSocket::listen(std::string sip, int sport, bool reuse_sport,
                                  int back_log) {
    error_t ret = st_tcp_listen(sip, sport, back_log, server_fd, reuse_sport);

    if (ret != SUCCESS) {
        sp_error("failed listen %s:%d, %d, %s", sip.c_str(), sport, back_log,
                  reuse_sport ? "true" : "false");
        return ret;
    }
    sp_debug("success listen %s:%d, backlog: %d, reuse: %u", sip.c_str(),
              sport, back_log, reuse_sport);

    this->backlog    = back_log;
    this->reuse_port = reuse_sport;
    this->port       = sport;

    return ret;
}

PSocket StTcpServerSocket::accept() {
    struct sockaddr_in addr;
    int len  = sizeof(addr);

    char buf[INET6_ADDRSTRLEN];
    memset(buf, 0, sizeof(buf));
    auto cfd = st_tcp_accept(server_fd, (struct sockaddr*) &addr,
                             &len, ST_UTIME_NO_TIMEOUT);

    std::string cip;
    int         cport = 0;
    addr2string(&addr, cip, cport);

    // discovery client information
    socklen_t len2 = sizeof(addr);
    if (getsockname(st_netfd_fileno(cfd), (sockaddr*) &addr, &len2) == -1) {
        return nullptr;
    }

    std::string sip;
    int         sport = 0;

    addr2string(&addr, sip, sport);

    sp_debug("accept client %s:%u, cfd:%d, from %s:%d -> %s:%d",
              std::string(buf, INET6_ADDRSTRLEN).c_str(),
              ntohs(addr.sin_port),
              st_tcp_fd(cfd), ip.c_str(), port,
              sip.c_str(), sport);

    auto io = std::make_shared<StTcpSocket>(cfd);
    return std::make_shared<Socket>(io, cip, cport, sip, sport);
}

}  // namespace sps

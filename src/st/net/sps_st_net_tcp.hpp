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

#ifndef SPS_ST_NET_TCP_HPP
#define SPS_ST_NET_TCP_HPP

extern "C" {
#include <public.h>
};

#include <string>

#include <net/sps_net_io.hpp>
#include <net/sps_net_socket.hpp>

namespace sps {

/**
 * dns 解析域名
 */
std::string dns_resolve(const std::string& host);

/**
 * st相关tcp的函数封装
 */
int           st_tcp_fd(st_netfd_t fd);
// 创建stfd，eg.用于tcp客户端上
error_t       st_tcp_create_fd(st_netfd_t& fd);
// 手动创建文件句柄fd，设置属性，需要生成对应的st句柄， eg. 主要用于服务器上
st_netfd_t    st_tcp_open_fd(int fd);
// 释放stfd，同时关闭文件句柄
error_t       st_tcp_close(st_netfd_t& fd);

error_t    st_tcp_listen(const std::string& server, int port,
                         int back_log, st_netfd_t& fd, bool reuse_port = true);
error_t    st_tcp_connect(const std::string& server, int port,
                          utime_t tm, st_netfd_t* fd);
st_netfd_t st_tcp_accept(st_netfd_t stfd, struct sockaddr *addr,
                         int *addrlen, utime_t timeout);
error_t    st_tcp_read(st_netfd_t stfd,  void *buf,
                       size_t nbyte, utime_t timeout);
error_t    st_tcp_write(st_netfd_t stfd, void *buf,
                        size_t nbyte, utime_t timeout);

error_t    st_tcp_writev(st_netfd_t stfd, const iovec *iov, int iov_size,
                         utime_t timeout, size_t *nwrite);

error_t    st_tcp_fd_reuseaddr(int fd, int enable = 1);
error_t    st_tcp_fd_reuseport(int fd);

class StTcpSocket : public IReaderWriter {
 public:
    explicit StTcpSocket(st_netfd_t stfd);
    ~StTcpSocket() override;

 public:
    // reader
    void    set_recv_timeout(utime_t tm) override;
    utime_t get_recv_timeout() override;

    error_t read_fully(void* buf, size_t size, ssize_t* nread) override;
    error_t read(void* buf, size_t size, size_t& nread) override;

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
    st_netfd_t stfd;
};

class StServerSocket : public IServerSocket {
 public:
    StServerSocket();
    ~StServerSocket();

 public:
    error_t listen(std::string sip, int sport, bool reuse_sport, int back_log);
    PSocket accept();

 private:
    std::string ip;
    int         port;
    bool        reuse_port;
    int         backlog;
    st_netfd_t  server_fd;
};

}

#endif  // SPS_ST_NET_TCP_HPP

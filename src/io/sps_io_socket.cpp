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

#include <sps_io_socket.hpp>

#include <memory>

#include <sps_log.hpp>
#include <sps_sys_st_io_srt.hpp>
#include <sps_sys_st_io_ssl.hpp>
#include <sps_sys_st_io_tcp.hpp>


namespace sps {

bool addr2string(const struct sockaddr_in* addr, std::string& ip, int& port) {
    char ip_addr[128];
    if (inet_ntop(addr->sin_family, (const void*) &addr->sin_addr, ip_addr, sizeof(ip_addr)) == nullptr) {
        ip_addr[0] = '\0';  // 防止内存写乱
        return false;
    }

    ip = std::string(ip_addr);
    port = ntohs(addr->sin_port);
    return true;
}

const char* get_transport_name(Transport t) {
    switch (t) {
        case TCP: return "tcp";
#ifdef SRT_ENABLED
        case SRT: return "srt";
#endif
        case QUIC: return "quic";
        case FILE: return "file";
#ifdef OPENSSL_ENABLED
        case HTTPS:  return "ssl";
#endif
        default: return "unknown";
    }
}

Socket::Socket() {
    init(nullptr, "", -1, "", -1);
}

Socket::Socket(PIReaderWriter rw, const std::string& ip, int p, const std::string& sip, int sp) {
    init(std::move(rw), ip, p, sip, sp);
}

void Socket::init(PIReaderWriter rw, const std::string& ip, int p, const std::string& sip, int sp) {
    this->io    = std::move(rw);
    this->peer_ip    = ip;
    this->peer_port  = p;
    this->self_ip    = sip;
    this->self_port  = sp;
}

error_t Socket::read_fully(void* buf, size_t size, ssize_t* nread) {
    error_t ret = SUCCESS;
    ssize_t n   = 0;
    size_t  nr  = 0;

    do {
        if ((ret = this->read((char*)buf+n, size-n, nr)) != SUCCESS) {
            return ret;
        }

        n += nr;
        nr = 0;
    } while (n < size);

    return ret;
}

error_t Socket::read(void* buf, size_t size, size_t& nread) {
    return io->read(buf, size, nread);
}

void Socket::set_recv_timeout(utime_t tm) {
    io->set_recv_timeout(tm);
}

utime_t Socket::get_recv_timeout() {
    return io->get_recv_timeout();
}

bool Socket::seekable() {
    return io->seekable();
}

error_t Socket::write(void* buf, size_t size) {
    return io->write(buf, size);
}

void Socket::set_send_timeout(utime_t tm)  {
    return io->set_send_timeout(tm);
}

utime_t Socket::get_send_timeout() {
    return io->get_send_timeout();
}

PIReaderWriter Socket::get_io() {
    return io;
}

std::string Socket::get_peer_ip() const {
    return peer_ip;
}
int  Socket::get_peer_port() const {
    return peer_port;
}

std::string Socket::get_self_ip() const {
    return self_ip;
}

int Socket::get_self_port() const {
    return self_port;
}

PSocket ClientSocketFactory::create_ss(
    Transport transport,
    const std::string &ip,
    int port, utime_t tm) {
    error_t ret = SUCCESS;

    switch (transport) {
        case Transport::TCP: {
            st_netfd_t fd;
            std::string peer_ip;
            if ((ret = st_tcp_connect(ip, port, tm, &fd, &peer_ip)) != SUCCESS) {
                return nullptr;
            }

            return std::make_shared<Socket>(std::make_shared<StTcpSocket>(fd), peer_ip, port);
        }

#ifdef SRT_ENABLED
        case Transport::SRT: {
            auto fd = st_srt_create_fd();

            if (fd == SRT_INVALID_SOCK) {
                sp_error("Failed create srt fd connect %s:%d, tm:%llu, ret:%d",
                         ip.c_str(), port, tm, ret);
                return nullptr;
            }

            if ((ret = st_srt_connect(fd, ip, port, tm)) != SUCCESS) {
                st_srt_close(fd);
                sp_error("Failed srt connect %s:%d, tm:%llu, ret:%d",
                         ip.c_str(), port, tm, ret);
                return nullptr;
            }

            return  std::make_shared<Socket>(
                    std::make_shared<StSrtSocket>(fd),
                    ip, port);
        }
#endif

#ifdef OPENSSL_ENABLED
        case Transport::HTTPS: {
            auto ss = create_ss(Transport::TCP, ip, port, tm);
            if (!ss) {
                return nullptr;
            }

            ss->set_send_timeout(tm);
            ss->set_recv_timeout(tm);

            SSLConfig config;
            config.server_host = ip;
            auto ssl = std::make_shared<StSSLSocket>(ss, SSLRole::SSL_CLIENT, config);
            if (ssl->init() != SUCCESS) {
                return nullptr;
            }

            return  std::make_shared<Socket>(ssl, ip, port);
        }
#endif
        default:
            return nullptr;
    }
}

PIServerSocket ServerSocketFactory::create_ss(Transport transport) {
    switch (transport) {
        case Transport::TCP:
            return std::make_shared<StTcpServerSocket>();

#ifdef SRT_ENABLED
        case Transport::SRT:
            return std::make_shared<StSrtServerSocket>();
#endif

#ifdef OPENSSL_ENABLED
        case Transport::HTTPS: {
            auto tcp = create_ss(Transport::TCP);
            if (!tcp) {
                return nullptr;
            }
            return std::make_shared<StSSLServerSocket>(tcp);
        }
#endif
        default:
            return nullptr;
    }
}

}  // namespace sps

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

//
// Created by byrcoder on 2021/8/23.
//

#include <sps_sys_st_io_udp.hpp>
#include <sps_io_socket.hpp>
#include <sps_log.hpp>

namespace sps {

error_t UdpFd::create_fd(PUdpFd &fd) {
    st_netfd_t net = nullptr;
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        return ERROR_SOCKET_CREATE;
    }

    net = st_netfd_open_socket(sock);
    if (net == nullptr) {
        ::close(sock);
        return ERROR_ST_OPEN_SOCKET;
    }

    fd = std::make_shared<UdpFd>(net);
    return SUCCESS;
}

error_t UdpFd::create_fd(int bind_port, PUdpFd &fd, bool reuse_addr, bool reuse_port) {
    error_t ret = create_fd(fd);

    if (ret != SUCCESS) {
        return ret;
    }

    int    fileno  = st_netfd_fileno(fd->get_fd());
    struct sockaddr_in addr;

    if ((ret = st_tcp_fd_reuseaddr(fileno)) != SUCCESS) {
        goto failed;
    }

    if (reuse_port) {
        if ((ret = st_tcp_fd_reuseport(fileno)) != SUCCESS) {
            goto failed;
        }
    }

    addr.sin_family        =   AF_INET;
    addr.sin_addr.s_addr   =   htonl(INADDR_ANY);
    addr.sin_port          =   htons(bind_port);

    if (::bind(fileno, (struct sockaddr*) &addr, sizeof(addr)) == -1) {
        sp_error("bind addr :%d failed, fileno:%d",
                  bind_port, fileno);
        ret = ERROR_SOCKET_BIND;
        goto failed;
    }

    return ret;

failed:
    fd.reset();
    return ret;
}

UdpFd::UdpFd(st_netfd_t udp_fd) {
    this->udp_fd = udp_fd;
}

UdpFd::~UdpFd() {
    st_tcp_close(udp_fd);
}

st_netfd_t UdpFd::get_fd() {
    return udp_fd;
}

StUdpClientSocket::StUdpClientSocket(const std::string& peer_ip, int peer_port, PUdpFd fd) {
    rtm    = -1;
    stm    = -1;
    rbytes = 0;
    sbytes = 0;

    this->fd        = std::move(fd);
    this->peer_port = peer_port;
    this->peer_ip   = peer_ip;
    this->connected = false;

    peer_addr.sin_family        =   AF_INET;
    peer_addr.sin_addr.s_addr   =   htonl(INADDR_ANY);
    peer_addr.sin_port          =   htons(peer_port);

    int opt = 1;
    setsockopt(st_netfd_fileno(this->fd->get_fd()), IPPROTO_IP, IP_PKTINFO, &opt, sizeof(opt));
}

// reader
void StUdpClientSocket::set_recv_timeout(utime_t tm) {
    this->rtm = tm;
}

utime_t StUdpClientSocket::get_recv_timeout() {
    return rtm;
}

error_t StUdpClientSocket::connect() {
    error_t  ret = st_connect(fd->get_fd(), reinterpret_cast<const sockaddr *>(&peer_addr), sizeof(peer_addr), 10);
    if (ret == 0) {
        connected = true;
    }

    return ret;
}

error_t StUdpClientSocket::read_fully(void* buf, size_t size, ssize_t* nread) {
    int     n   = 0;
    error_t ret = SUCCESS;

    while (n < size) {
        size_t nr = 0;
        if ((ret = read((char*) buf + n, size - n, nr)) != SUCCESS) {
            return ret;
        }

        n += nr;
    }

    return ret;
}

error_t StUdpClientSocket::read(void* buf, size_t size, size_t& nread) {
    std::string peer_ip;
    int         peer_port;
    return read(buf, size, nread, peer_ip, peer_port);
}

error_t StUdpClientSocket::read(void *buf, size_t size, size_t &nread,
                                std::string &peer_ip, int &peer_port) {
    struct sockaddr_in from_addr;
    int from_len = sizeof(from_addr);

    int n = st_recvfrom(fd->get_fd(), buf, size, (struct sockaddr*) &from_addr, &from_len, rtm);

    sp_debug("st_recvfrom ret %d", n);

    if (n <= 0) {
        if (n < 0 && errno == ETIME) {
            return ERROR_SOCKET_TIMEOUT;
        }

        if (errno == EINTR) {
            return ERROR_SOCKET_EINTR;
        }
        return ERROR_SOCKET_READ;
    }

    addr2string(&from_addr, peer_ip, peer_port);
    nread = n;
    return SUCCESS;
}

error_t StUdpClientSocket::read(void *buf, size_t size, size_t &nread,
                                std::string &peer_ip, int &peer_port,
                                std::string &dst_ip, int &dst_port) {
    struct sockaddr_in from_addr;
    int from_len = sizeof(from_addr);

    struct iovec io;
    io.iov_base = buf;
    io.iov_len  = size;

    struct msghdr msg = {
        .msg_name = &from_addr,
        .msg_namelen = sizeof(from_addr),
        .msg_iov = &io,
        .msg_iovlen = 1,
        .msg_control = control_buf,
        .msg_controllen = sizeof(control_buf),
        .msg_flags = 0
    };

    int n = st_recvmsg(fd->get_fd(), &msg, 0, rtm);

    if (n < 0) {
        if (n < 0 && errno == ETIME) {
            return ERROR_SOCKET_TIMEOUT;
        }

        if (errno == EINTR) {
            return ERROR_SOCKET_EINTR;
        }
        return ERROR_SOCKET_READ;
    }

    // iterate through all the control headers
    for (struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg); cmsg != nullptr;
        cmsg = CMSG_NXTHDR(&msg, cmsg)) {
        // ignore the control headers that don't match what we want
        if (cmsg->cmsg_level != IPPROTO_IP || cmsg->cmsg_type != IP_PKTINFO) {
            continue;
        }
        struct in_pktinfo *pi = (struct in_pktinfo*) CMSG_DATA(cmsg);
        // at this point, peeraddr is the source sockaddr
        // pi->ipi_spec_dst is the destination in_addr
        // pi->ipi_addr is the receiving interface in_addr

        addr2string((struct sockaddr_in *)&pi->ipi_addr, dst_ip, dst_port);
    }

    nread = n;
    return SUCCESS;
}

// writer
void StUdpClientSocket::set_send_timeout(utime_t tm) {
    this->stm = tm;
}

utime_t StUdpClientSocket::get_send_timeout() {
    return stm;
}

error_t StUdpClientSocket::write(void* buf, size_t size) {
    int sent = 0;

    while (sent < size) {
        int n = st_sendto(fd->get_fd(), (char*) buf + sent, size - sent,
                          connected ? nullptr : (struct sockaddr *) &peer_addr, sizeof(peer_addr), stm);

        if (n < 0) {
            if (n < 0 && errno == ETIME) {
                return ERROR_SOCKET_TIMEOUT;
            }

            if (errno == EINTR) {
                return ERROR_SOCKET_EINTR;
            }
            return ERROR_SOCKET_WRITE;
        }

        sent += n;
    }

    return SUCCESS;
}

UdpBuffer::UdpBuffer(uint8_t *buf, size_t len) {
    this->len = len;
    this->buf = new uint8_t[len];
    memcpy(this->buf, buf, len);
}

UdpBuffer::~UdpBuffer() {
    delete [] buf;
}

UdpBuffer::UdpBuffer(const UdpBuffer &buffer) {
}

uint8_t * UdpBuffer::buffer() {
    return buf;
}

size_t UdpBuffer::size() {
    return len;
}

StUdpSessionClientSocket::StUdpSessionClientSocket(const std::string &cip, int cport, PUdpFd fd) :
        StUdpClientSocket(cip, cport, fd) {
    cond             = IConditionFactory::get_instance().create_condition();
    buffer_list_size = 0;
    buffer_max_list_size = 4000;
}

StUdpSessionClientSocket::~StUdpSessionClientSocket() {
}

error_t StUdpSessionClientSocket::read(void *buf, size_t size, size_t &nread) {
    if (buffer_lists.empty()) {
        cond->wait(get_recv_timeout());
    }

    if (buffer_lists.empty()) {
        return ERROR_SOCKET_TIMEOUT;
    }

    auto& b = buffer_lists.front();

    if (size < b->size()) {
        return ERROR_SOCKET_BUFFER_SMALL;
    }

    nread = b->size();
    memcpy(buf, b->buffer(), b->size());

    buffer_lists.erase(buffer_lists.begin());
    --buffer_list_size;

    return SUCCESS;
}

error_t StUdpSessionClientSocket::push(uint8_t* buf, size_t len) {
    bool empty = buffer_lists.empty();

    if (buffer_list_size >= buffer_max_list_size) {
        sp_warn("too fast udp buffer_list_size %d drop", buffer_list_size);
        return SUCCESS;
    }

    buffer_lists.push_back(std::make_shared<UdpBuffer>(buf, len));

    if (empty) {
        cond->signal();
    }
    ++buffer_list_size;

    return SUCCESS;
}

UdpSessionSocket::UdpSessionSocket(IUdpSocketManager* manger): Socket() {
    this->manager = manger;
}

UdpSessionSocket::UdpSessionSocket(IUdpSocketManager* manager, PIReaderWriter rw,
        const std::string& ip, int p, const std::string& dip, int dp) : Socket(rw, ip, p, dip, dp) {
    this->manager = manager;
}

UdpSessionSocket::~UdpSessionSocket() {
    manager->on_destroy(this);
}

StUdpServerSocket::~StUdpServerSocket() {
    delete [] buffer;
}

error_t StUdpServerSocket::listen(std::string sip, int sport, bool reuse_sport, int /** back_log **/ ) {
    this->ip   = sip;
    this->port = sport;
    this->reuse_port = reuse_sport;
    new_cond         = IConditionFactory::get_instance().create_condition();

    error_t ret = UdpFd::create_fd(sport, server_fd, reuse_port, reuse_sport);

    if (ret != SUCCESS) {
        sp_error("create udp server socket failed ret %d", ret);
        return ret;
    }

    server_socket = std::make_shared<StUdpClientSocket>("", 0, server_fd);

    delete [] buffer;
    buffer        = new uint8_t[buffer_size];
    sp_info("create udp server socket success port %d", sport);

    ret           = ICoFactory::get_instance().start(shared_from_this());
    if (ret != SUCCESS) {
        sp_error("start udp server socket failed ret %d", ret);
        return ret;
    }

    running       = true;
    return ret;
}

PSocket StUdpServerSocket::accept() {
    while (new_clients.empty()) {
        new_cond->wait(SLEEP_FOREVER);
    }

    PSocket c = new_clients.front();
    new_clients.pop_front();
    sp_trace("accept %s:%d->%s:%d", c->get_peer_ip().c_str(), c->get_peer_port(),
             c->get_self_ip().c_str(), c->get_self_port());
    return c;
}

error_t StUdpServerSocket::handler() {
    while (running) {
        size_t      nr    = 0;
        std::string peer_ip;
        int         peer_port = 0;
        std::string self_ip;
        int         self_port = 0;
        bool        new_client = false;

        error_t ret = server_socket->read(buffer, buffer_size, nr, peer_ip, peer_port, self_ip, self_port);
        if (ret != SUCCESS){
            sp_error("fail read ret %d", ret);
            continue;;
        }

        PStUdpSessionClientSocket csocket = search_or_create(peer_ip, peer_port, self_ip, self_port, new_client);
        csocket->push(buffer, nr);
        // new socket
        if (new_client) {
            new_clients.push_back(std::make_shared<UdpSessionSocket>(this, csocket, peer_ip, peer_port, self_ip, self_port));
            new_cond->signal();
        }
    }

    return SUCCESS;
}

void StUdpServerSocket::on_destroy(UdpSessionSocket* session) {
    remove(session->get_peer_ip(), session->get_peer_port(), session->get_self_ip(), session->get_self_port());
}

std::string StUdpServerSocket::generate_key(const std::string& cip, int cport,
                                            const std::string& dip, int dport) {
    return cip + ":" + std::to_string(cport) + "->" + dip + std::to_string(dport);
}

PStUdpSessionClientSocket StUdpServerSocket::search_or_create(std::string& cip, int cport,
        std::string& dip, int dport, bool &is_new) {
    std::string key = generate_key(cip, cport, dip, dport);

    auto it = sessions.find(key);
    if (it != sessions.end()) {
        return it->second.lock();
    }

    auto new_socket = std::make_shared<StUdpSessionClientSocket> (cip, cport, server_fd);
    sessions[key] = std::weak_ptr<StUdpSessionClientSocket>(new_socket);
    is_new = true;

    sp_trace("udp client coming %s:%d", cip.c_str(), cport);

    return new_socket;
}

void StUdpServerSocket::remove(const std::string& cip, int cport, const std::string& sip, int sport) {
    std::string key = generate_key(cip, cport, sip, sport);

    sp_trace("udp client leaving %s:%d", cip.c_str(), cport);
    sessions.erase(key);
}

}
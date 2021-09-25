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

#include <sps_st_io_srt.hpp>

#include <sps_st_io_tcp.hpp>
#include <sps_log.hpp>

#ifdef SRT_ENABLED

#include <string>
#include <vector>
#include <srtcore/udt.h>
#include <sstream>


namespace sps {

static StSrtMode default_srt_mode = ST_SRT_MODE_LIVE;

int st_srt_epoll(SRTSOCKET fd, StSrtEvent evt, utime_t tm, std::string& err) {
    int ret = SrtStDispatch::get_instance()->st_srt_epoll(fd, evt, tm, err);

    if (ret != 0) {
        sp_error("srt epoll failed fd:%d, ret:%d, err:%s", fd, ret, err.c_str());
        return ret;
    }

    return ret;
}

int st_srt_socket_error(SRT_SOCKSTATUS status) {
    switch (status) {
        case SRTS_BROKEN:
            return ERROR_SRT_SOCKET_BROKEN;
        case SRTS_CLOSING:
            return ERROR_SRT_SOCKET_CLOSING;
        case SRTS_CLOSED:
            return ERROR_SRT_SOCKET_CLOSED;
        case SRTS_NONEXIST:
            return ERROR_SRT_SOCKET_NOTEXITS;
        default:
            return ERROR_SRT_SOCKET_STATUS_OK;
    }
}

int st_srt_set_global_mode(StSrtMode mode) {
    if (mode == ST_SRT_MODE_LIVE) {
        default_srt_mode = ST_SRT_MODE_LIVE;
    } else {
        default_srt_mode = ST_SRT_MODE_FILE;
    }
    return SUCCESS;
}

SRTSOCKET st_srt_create_fd(StSrtMode mode) {
    auto fd      = srt_create_socket();
    bool syn     = false;
    bool drop    = false;
    bool tspd    = false;

    if (fd == SRT_INVALID_SOCK) {
        sp_error("[st_srt_create]: srt create socket  failed");
        return fd;
    }

    srt_setsockflag(fd, SRTO_RCVSYN, &syn, sizeof(syn));
    srt_setsockflag(fd, SRTO_SNDSYN, &syn, sizeof(syn));

    // set default mode
    if (mode == ST_SRT_MODE_GLOBAL) {
        mode = default_srt_mode;
    }

    if (mode == ST_SRT_MODE_FILE) {
        SRT_TRANSTYPE t = SRTT_FILE;
        srt_setsockflag(fd, SRTO_TRANSTYPE, &t, sizeof(t));

        linger lin = { 2, 1 };

        srt_setsockopt(fd, 0, SRTO_LINGER, &lin, sizeof(lin));

        // rapid nak
        bool nak = true;
        srt_setsockflag(fd, SRTO_NAKREPORT, &nak, sizeof(nak));
    } else {
        SRT_TRANSTYPE t = SRTT_LIVE;
        srt_setsockflag(fd, SRTO_TRANSTYPE, &t, sizeof(t));

        int send_delay = -1;
        srt_setsockflag(fd, SRTO_TSBPDMODE, &tspd, sizeof(tspd));
        srt_setsockflag(fd, SRTO_SNDDROPDELAY, &send_delay, sizeof(send_delay));
        srt_setsockflag(fd, SRTO_TLPKTDROP, &drop, sizeof(drop));

        linger lin = { 3, 0 };
        srt_setsockopt(fd, 0, SRTO_LINGER, &lin, sizeof(lin));
    }

    return fd;
}

error_t  st_srt_connect(SRTSOCKET fd, const std::string& server,
                        int port, utime_t tm, UDPSOCKET ufd, bool one_or_zero_rtt,
                        const char* req, int req_len) {
    struct sockaddr_in target_addr{};

    target_addr.sin_family = AF_INET;
    target_addr.sin_port   = htons(port);
    int ret                = SUCCESS;

    std::string server_ip  = dns_resolve(server);

    if (server_ip.empty()) {
        sp_error("st_srt_connect: dns_resolve addr failed, %s:%d", server.c_str(), port);
        return ERROR_SYSTEM_IP_INVALID;
    }

    if (inet_pton(AF_INET, server_ip.data(), &target_addr.sin_addr) != 1) {
        sp_error("st_srt_connect: srt create socket addr failed, %s:%d", server.c_str(), port);
        return ERROR_SRT_SOCKET_CREATE;
    }

    if (ufd > 0) {
        if ((ret = srt_bind_acquire(fd, ufd)) == SRT_ERROR) {
            sp_error("[srt_bind_acquire] failed ret:%d", ret);
            return ERROR_SRT_SOCKET_BIND;
        }
    }

    ret = srt_connect(fd, (struct sockaddr*) &target_addr, sizeof(target_addr));

    if (ret == SRT_ERROR) {
        sp_error("[st_srt_connect]: srt create socket addr failed, %s:%d", server.c_str(), port);
        return ERROR_SRT_SOCKET_CONNECT;
    }

    std::string err;
    ret = st_srt_epoll(fd, ST_SRT_CONN, tm, err);

    if (ret != 0) {
        sp_error("[st_srt_connect]: srt create socket addr failed, %s:%d, "
                   "one_rtt_conf:%d",
                 server.c_str(), port, one_or_zero_rtt);
        return ret;
    }

    sp_info("[st_srt_connect]: srt create socket addr failed, %s:%d, "
              " one_rtt_conf:%d",
            server.c_str(), port, one_or_zero_rtt);

    return ret;
}

error_t st_srt_connect(SRTSOCKET fd, const std::string& server, int port,
                       const std::string& self_ip, int self_port, utime_t tm,
                       bool one_or_zero_rtt,
                       const char* zero_req,
                       int zero_req_len) {
    struct sockaddr_in target_addr{};
    target_addr.sin_family = AF_INET;
    target_addr.sin_port   = htons(port);

    std::string server_ip  = dns_resolve(server);

    if (server_ip.empty()) {
        sp_error("st_srt_connect: dns_resolve addr failed, %s:%d", server.c_str(), port);
        return ERROR_SYSTEM_IP_INVALID;
    }

    if (inet_pton(AF_INET, server_ip.data(), &target_addr.sin_addr) != 1) {
        sp_error("st_srt_connect: srt create socket addr failed, %s:%d", server.c_str(), port);
        return ERROR_SRT_SOCKET_CREATE;
    }

    struct sockaddr_in self_addr{};
    self_addr.sin_family = AF_INET;
    self_addr.sin_port   = htons(self_port);

    if (inet_pton(AF_INET, self_ip.data(), &self_addr.sin_addr) != 1) {
        sp_error("st_srt_connect: srt create socket addr failed, %s:%d", self_ip.c_str(), self_port);
        return ERROR_SRT_SOCKET_CREATE;
    }

    int ret = SUCCESS;

    bool one_rtt_ok = false;

    if (zero_req != nullptr && zero_req_len > 0) {
#ifndef  SRT_ZERO_RRT_ENABLED
        sp_error("[st_srt_connect] not support 0rrt, req must be null");
        return ERROR_SRT_NOT_SUPPORT_FAST_OPEN;
#endif
        if (!one_or_zero_rtt) {
            sp_error("[st_srt_connect] support 0rrt but config not set one_or_zero_rtt");
            return ERROR_SRT_NOT_SUPPORT_FAST_OPEN;
        }
    }

#ifdef SRT_ONE_RRT_ENABLED
    one_rtt_ok = true;
    if (one_or_zero_rtt) {
        if (zero_req && zero_req_len > 0) {
            if (!srt::set0rttdata(fd, std::string(zero_req, zero_req_len))) {
                sp_error("set srt data failed req_len:%d", zero_req_len);
                return ERROR_NOT_SUPPORT_SRT_FAST_OPEN;
            }
        }

        ret = srt_fast_connect_bind(fd, reinterpret_cast<const struct sockaddr*>(&self_addr),
                                    reinterpret_cast<const struct sockaddr*>(&target_addr),
                                    sizeof(self_addr));
    } else {
        ret = srt_connect_bind(fd, reinterpret_cast<const struct sockaddr*>(&self_addr),
                               reinterpret_cast<const struct sockaddr*>(&target_addr),
                               sizeof(self_addr));
    }
#else
    ret = srt_connect_bind(fd, (const struct sockaddr*) &self_addr,
            (const struct sockaddr*) &target_addr, sizeof(self_addr));
#endif

    if (ret == SRT_ERROR) {
        sp_error("[st_srt_connect2]: srt create socket addr failed, target:"
                   " %s:%d, self:%s:%d, one_rtt_ok:%d,"
                   "one_rtt_conf:%d ",
                 server.c_str(), port, self_ip.c_str(), self_port, one_rtt_ok, one_or_zero_rtt);
        return ERROR_SRT_SOCKET_CONNECT;
    }

    std::string err;
    ret = st_srt_epoll(fd, ST_SRT_CONN, tm, err);

    if (ret != 0) {
        sp_error("[st_srt_connect2]: srt connect socket addr failed, server:%s, port:%d, "
                   "ret:%d, err:%s", server.c_str(), port, ret, err.c_str());
        return ret;
    }

    sp_info("[st_srt_connect2]: srt create socket addr failed, target: "
              "%s:%d, self:%s:%d, one_rtt_ok:%d,"
              "one_rtt_conf:%d ",
            server.c_str(), port, self_ip.c_str(), self_port, one_rtt_ok, one_or_zero_rtt);

    return ret;
}

extern error_t st_tcp_fd_reuseport(int fd);

error_t st_srt_listen(SRTSOCKET fd, const std::string& server, int port,
                      int back_log, bool reuse_port) {
    struct sockaddr_in self_addr{};

    self_addr.sin_family = AF_INET;
    self_addr.sin_port   = htons(port);

    do {
        UDPSOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

        if (sock == -1) {
            sp_error("udp socket create failed sock:%d", sock);
            goto failed;
        }

        if (reuse_port) {
            if (st_tcp_fd_reuseaddr(sock) != SUCCESS) {
                sp_error("reuse addr failed port:%d", port);
                goto failed;
            }

            if (st_tcp_fd_reuseport(sock) != SUCCESS) {
                sp_error("reuse port failed port:%d", port);
                goto failed;
            }
        }

        if (::bind(sock, reinterpret_cast<struct sockaddr*>(&self_addr),
                sizeof(self_addr)) == -1) {
            sp_error("udp socket create failed sock:%d", sock);
            goto failed;
        }

        if (srt_bind_acquire(fd, sock) == SRT_ERROR) {
            sp_error("[srt_bind]: socket addr failed, server:%s, port:%d", server.c_str(), port);
            goto failed;
        }

        break;
failed:
        close(sock);

        if (reuse_port) {
            return ERROR_SRT_SOCKET_CREATE;
        }

        if (srt_bind(fd, reinterpret_cast<struct sockaddr*>(&self_addr),
                sizeof(self_addr) == SRT_ERROR)) {
            sp_error("[srt_bind]: socket addr failed, server:%s, port:%d", server.c_str(), port);
            return ERROR_SRT_SOCKET_BIND;
        }
    } while (0);

    if (srt_listen(fd, back_log) == SRT_ERROR) {
        sp_error("[srt_listen]: socket addr failed, server:%s, port:%d", server.c_str(), port);
        return ERROR_SRT_SOCKET_LISTEN;
    }

    return SUCCESS;
}

SRTSOCKET st_srt_accept(SRTSOCKET fd, struct sockaddr *addr, int *addrlen, utime_t timeout) {
    std::string err;
    SRTSOCKET ret = SRT_INVALID_SOCK;

    while ((ret = srt_accept(fd, addr, addrlen)) == SRT_INVALID_SOCK) {
        int eno = 0;

        if (st_srt_again(eno)) {
            ret = st_srt_epoll(fd, ST_SRT_ACCEPT, timeout, err);

            if (ret != SUCCESS) {
                sp_error("[st_srt_accept]: srt epoll failed, fd:%d, "
                           "ret:%d, err:%s", fd, ret, err.c_str());
                return ret;
            }
        } else {
            sp_error("[st_srt_accept] failed, fd:%d, ret:%d, err:%s", fd, ret, err.c_str());
            return ret;
        }
    }
    return ret;
}

error_t st_srt_read(SRTSOCKET fd, void *buf, size_t nbyte, utime_t timeout, size_t& nread) {
    std::string err;
    int ret = SUCCESS;

    while ((ret = srt_recv(fd, reinterpret_cast<char*>(buf), nbyte)) == SRT_ERROR) {
        int eno = 0;
        if (st_srt_again(eno)) {
            ret = st_srt_epoll(fd, ST_SRT_IN, timeout, err);
            if (ret != SUCCESS) {
                sp_error("[st_srt_read]: srt read epoll failed, fd:%d, "
                           "ret:%d, err:%s", fd, ret, err.c_str());
                return ret;
            }
        } else {
            err = srt_getlasterror_str();
            sp_error("[st_srt_read]: srt read socket addr failed, fd:%d, "
                       "srt_errno:%d, ret:%d, err:%s", fd, eno, ret, err.c_str());
            return ret;
        }
    }

    if (ret <= 0) {
        err = srt_getlasterror_str();
        sp_error("[st_srt_read]: srt read socket addr failed, fd:%d, "
                   "ret:%d, err:%s", fd, ret, err.c_str());
        return ret == 0 ? ERROR_SRT_SOCKET_CLOSED : ERROR_SRT_SOCKET_UNKNOWN;
    }

    nread = ret;
    return SUCCESS;
}

error_t st_srt_write(SRTSOCKET fd, void *buf, size_t nbyte, utime_t timeout) {
    std::string err;
    int ret = SUCCESS;

    size_t has_send = 0;

    // srt 的写限制
    while (has_send < nbyte) {
        size_t next_send = nbyte - has_send;

        if (next_send > SRT_LIVE_DEF_PLSIZE) {
            next_send = SRT_LIVE_DEF_PLSIZE;
        }

        while ((ret = srt_sendmsg(fd, reinterpret_cast<char*>(buf) + has_send, next_send,
                 -1, true)) == SRT_ERROR) {
            int eno = 0;
            if (st_srt_again(eno)) {
                ret = st_srt_epoll(fd, ST_SRT_OUT, timeout, err);
                if (ret != SUCCESS) {
                    sp_error("[st_srt_write]: srt write epoll failed, fd:%d, "
                               "ret:%d, err:%s", fd, ret, err.c_str());
                    return ret;
                }
            } else {
                err = srt_getlasterror_str();
                sp_error("[st_srt_write]: srt write socket addr failed, fd:%d,"
                           " errno:%d, err:%s", fd, eno, err.c_str());
                return ret;
            }
        }
        has_send += ret;
    }

    return SUCCESS;
}

error_t st_srt_close(SRTSOCKET& fd) {
    // TODO(weideng): FIXME HERE WITH LINGER
    // 延迟关闭, srt需要提供是否写完了fd的配置
    // st_usleep(3 * 1000 * 1000);
    srt_close(fd);
    sp_info("close srt fd:%d", fd);
    fd = SRT_INVALID_SOCK;
    return SUCCESS;
}

bool st_srt_again(int &err) {
    err = srt_getlasterror(nullptr);
    return err / 1000 == MJ_AGAIN;
}

int st_srt_stat(SRTSOCKET fd, std::string& res, SRT_TRACEBSTATS* perf) {
    int ret = srt_bstats(fd, perf, 1);
    if (ret == SRT_ERROR) {
        return ret;
    }

    return st_srt_stat_print(res, perf);
}

int st_srt_stat_print(std::string& res, SRT_TRACEBSTATS* perf) {
    std::stringstream in;

    // 重传率
    double re_send = st_srt_stat_retrans(perf, true);
    double re_rcv  = st_srt_stat_retrans(perf, false);

    // 丢包率
    double send_loss = st_srt_stat_loss(perf, true);
    double recv_loss = st_srt_stat_loss(perf, false);

    in  << "rcv_drop_bytes: "  <<  perf->byteRcvDrop << "\t"
        << "send_loss_bytes: " <<  perf->byteSndDrop << "\t"
        << "send_re: "         <<  re_send   << "\t"
        << "rcv_re: "          <<  re_rcv    << "\t"
        << "send_loss: "       <<  send_loss << "\t"
        << "recv_loss: "       <<  recv_loss << "\t"
        << "send_app_bytes: "  <<  perf->byteSentUnique << "\t"
        << "rcv_app_bytes: "   <<  perf->byteRecvUnique << "\t"
        << "send_total_bytes: " <<  perf->byteSent << "\t"
        << "rcv_total_bytes: "  <<  perf->byteRecv;

    res =  in.str();
    return SUCCESS;
}

double st_srt_stat_loss(SRT_TRACEBSTATS* perf, bool is_send) {
    if (is_send) {
        return (perf->byteSentUnique > 0) ?
            perf->byteSndDrop * 10000.0 / perf->byteSentUnique : 0.0;
    } else {
        return (perf->byteRecvUnique > 0) ?
            perf->byteRcvDrop * 10000.0 / perf->byteRecvUnique : 0.0;
    }
}

double st_srt_stat_retrans(SRT_TRACEBSTATS* perf, bool is_send) {
    if (is_send) {
        return (perf->byteSentUnique > 0) ?
            (perf->byteSent - perf->byteSentUnique) * 10000.0 / perf->byteSentUnique : 0.0;
    } else {
        return (perf->byteRecvUnique > 0) ?
            (perf->byteRecv - perf->byteRecvUnique) * 10000.0 / perf->byteRecvUnique : 0.0;
    }
}

SrtEventCondition::SrtEventCondition(StSrtEvent evt) {
    cond        = st_cond_new();
    event       = evt;
    err         = 0;
}

SrtEventCondition::~SrtEventCondition() {
    st_cond_destroy(cond);
}

SrtStDispatch* SrtStDispatch::get_instance() {
    thread_local std::shared_ptr<SrtStDispatch> srt_dispatch;
    if (srt_dispatch.get() == nullptr) {
        srt_dispatch = std::make_shared<SrtStDispatch>();
        srt_dispatch->init();
    }
    return srt_dispatch.get();
}

SrtStDispatch::SrtStDispatch() {
    stop       = false;
    srt_eid    = -1;
}

SrtStDispatch::~SrtStDispatch() {
}

int SrtStDispatch::init() {
    srt_eid = srt_epoll_create();

    srt_epoll_set(srt_eid, SRT_EPOLL_ENABLE_EMPTY);
    return sps::ICoFactory::get_instance().start(shared_from_this());
}

int SrtStDispatch::st_srt_epoll(SRTSOCKET fd, StSrtEvent event, utime_t timeout, std::string& err) {
    int evt = 0;

    auto it  = conditions.find(fd);
    bool add;
    int  ret = SUCCESS;

    if (it == conditions.end()) {
        add   = true;
        evt   = event | ST_SRT_ERR;
    } else {
        int old_evt = get_exits_evt(fd);
        add         = false;
        evt         = old_evt | event | ST_SRT_ERR;
    }

    if (add) {
        if (UDT::epoll_add_usock(srt_eid, fd, &evt) == SRT_ERROR) {
            return SRT_ERROR;
        }
    } else {
        evt |= SRT_EPOLL_ETONLY;  // et only
        if (UDT::epoll_update_usock(srt_eid, fd, &evt) == SRT_ERROR) {
            return SRT_ERROR;
        }
    }

    // st wait util timeout or notify
    auto cnd = std::make_shared<SrtEventCondition>(event);
    conditions[fd].insert(cnd);
    ret = st_cond_timedwait(cnd->cond, timeout);

    if (conditions.count(fd))
        conditions[fd].erase(cnd);

    int old_evt = get_exits_evt(fd);

    if (old_evt == 0) {
        // remove all event when nothing happend
        if (UDT::epoll_remove_usock(srt_eid, fd) == SRT_ERROR) {
            sp_error("remove fd failed ingore it fd:%d", fd);
        }
    } else {
        old_evt |= SRT_EPOLL_ETONLY;
        // try remove self evt
        if (UDT::epoll_update_usock(srt_eid, fd, &old_evt) == SRT_ERROR) {
            sp_error("update fd failed ingore it fd:%d", fd);
        }
    }

    if (cnd->err) {
        err = cnd->err_msg;
        return cnd->err;
    }

    if (ret != 0) {
        if (errno == ETIME) {
            return ERROR_SRT_SOCKET_TIMEOUT;
        } else if (errno == EINTR) {
            return ERROR_SOCKET_EINTR;
        }
        return ERROR_SRT_SOCKET_UNKNOWN;
    }

    return ret;
}

int SrtStDispatch::get_exits_evt(SRTSOCKET fd) {
    int old_evt = 0;
    auto it  = conditions.find(fd);

    if (it == conditions.end()) {
        return old_evt;
    }

    for (auto e : it->second) {
        old_evt |=  e->event;
    }
    return old_evt;
}

error_t SrtStDispatch::handler()  {
    const int len = 1024;
    std::vector<SRT_EPOLL_EVENT> events(len);
    while (!stop) {
        int es = UDT::epoll_uwait(srt_eid, &events[0], len, 0);

        for (int i = 0; i < es; ++i) {
            SRT_EPOLL_EVENT& event = events[i];
            auto it = conditions.find(event.fd);

            // empty event removed from events
            if (it == conditions.end()) {
                sp_error("unknow srt_fd:%d", event.fd);
                UDT::epoll_remove_ssock(srt_eid, event.fd);
            } else {
                auto& sets = it->second;
                int  err = 0;
                const char* err_msg = nullptr;

                if (event.events & StSrtEvent::ST_SRT_ERR) {
                    SRT_SOCKSTATUS status = srt_getsockstate(event.fd);
                    err_msg               = srt_getlasterror_str();
                    err                   = st_srt_socket_error(status);
                    sp_error("epoll error srt_fd:%d status:%d, err:%d", event.fd, status, err);
                }

                for (auto e = sets.begin(); e != sets.end(); ) {
                    PSrtEventCondition cnd = *e;
                    bool notify = false;

                    // error event notify all the socket
                    if (event.events & StSrtEvent::ST_SRT_ERR) {
                        cnd->err      = err;
                        cnd->err_msg  = err_msg;
                        notify = true;
                    } else {
                        if (event.events & cnd->event) {
                            notify = true;
                        }
                    }

                    if (notify) {
                        sets.erase(e++);
                        st_cond_broadcast(cnd->cond);
                    } else {
                        e++;
                    }
                }

                if (it->second.empty()) {
                    conditions.erase(it);
                }
            }
        }
        st_usleep(10 * 1000);
    }
    return SUCCESS;
}

StSrtSocket::StSrtSocket(SRTSOCKET fd) : fd(fd) {
    rtm = ST_UTIME_NO_TIMEOUT;
    stm = ST_UTIME_NO_TIMEOUT;

    read_buf  = std::make_unique<AVBuffer>(SRT_LIVE_DEF_PLSIZE, false);
    write_buf = std::make_unique<AVBuffer>(SRT_LIVE_DEF_PLSIZE, false);
}

StSrtSocket::~StSrtSocket() {
    sp_trace("srt socket close fd: %d", fd);
    st_srt_close(fd);
}

void StSrtSocket::set_recv_timeout(utime_t tm) {
    rtm = tm;
}

utime_t StSrtSocket::get_recv_timeout() {
    return rtm;
}

error_t StSrtSocket::read_fully(void* buf, size_t size, ssize_t* nread) {
    size_t n     = 0;
    int    ret   = SUCCESS;
    while (n < size) {
        size_t nr = 0;
        ret       = this->read(buf, size-n, nr);
        if (ret != SUCCESS) {
            if (nread) {
                *nread = n;
            }
            return ret;
        }
        n += nr;
    }
    if (nread) {
        *nread = n;
    }
    return SUCCESS;
}

error_t StSrtSocket::read(void* buf, size_t size, size_t& nread) {
    if (read_buf->size() > 0) {
        nread = std::min(size, read_buf->size());
        memcpy(buf,  read_buf->pos(), nread);
        read_buf->skip(nread);

        if (read_buf->size() == 0) {
            read_buf->clear();
        }

        return SUCCESS;
    }

    error_t ret = SUCCESS;

    // in case buffer < SRT_LIVE_DEF_PLSIZE error
    if (size < SRT_LIVE_DEF_PLSIZE) {
        size_t nr = 0;
        ret = st_srt_read(fd, read_buf->buffer(), read_buf->cap_size(), rtm, nr);
        if (ret != SUCCESS || nr == 0) {
            sp_error("srt read failed ret:%d, nr %ld", ret, nr);
            return ret;
        }

        read_buf->append(nr);
        return read(buf, size, nread);
    }

    ret = st_srt_read(fd, buf, size, rtm, nread);
    if (ret != SUCCESS) {
        sp_error("srt read failed ret:%d", ret);
        return ret;
    }
    return ret;
}

void StSrtSocket::set_send_timeout(utime_t tm) {
    stm = tm;
}

utime_t StSrtSocket::get_send_timeout() {
    return stm;
}

error_t StSrtSocket::write(void* buf, size_t size) {
    return st_srt_write(fd, buf, size, stm);
}

SRTSOCKET StSrtSocket::get_fd() {
    return fd;
}

std::string StSrtSocket::get_streamid() {
    return UDT::getstreamid(fd);
}

StSrtServerSocket::StSrtServerSocket() {
    server_fd = SRT_INVALID_SOCK;
}

StSrtServerSocket::~StSrtServerSocket() {
    st_srt_close(server_fd);
}

error_t StSrtServerSocket::listen(std::string ip, int port, bool reuse_port, int backlog) {
    server_fd = st_srt_create_fd(ST_SRT_MODE_LIVE);

    if (server_fd == SRT_INVALID_SOCK) {
        sp_error("fail create srt fd %d", server_fd);
        return ERROR_ST_OPEN_SOCKET;
    }

    return st_srt_listen(server_fd, ip, port, backlog, reuse_port);
}

PSocket StSrtServerSocket::accept() {
    struct sockaddr_in addr;
    int addr_len = sizeof(addr);
    char buf[INET6_ADDRSTRLEN];
    memset(buf, 0, sizeof(buf));

    SRTSOCKET c_fd = st_srt_accept(server_fd, (struct sockaddr*) &addr,
            &addr_len, ST_UTIME_NO_TIMEOUT);
    auto srt_socket = std::make_shared<StSrtSocket>(c_fd);

    if (inet_ntop(addr.sin_family, &addr.sin_addr, buf, sizeof(buf)) == NULL) {
        buf[0] = '\0';  // 防止内存写乱
    }

    return std::make_shared<Socket>(srt_socket, std::string(buf), addr.sin_port);
}

}

#endif


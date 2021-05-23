#ifndef SPS_CO_ST_SRT_HPP_
#define SPS_CO_ST_SRT_HPP_

#include <sps_auto_header.hpp>

#ifndef SRT_DISABLED

extern "C" {
#include <public.h>
};
#include <srtcore/srt.h>

#include <map>
#include <set>
#include <string>

#include "co/st/co.hpp"
#include "net/io.hpp"

namespace sps {

enum StSrtEvent {
    ST_SRT_IN = SRT_EPOLL_OPT::SRT_EPOLL_IN,
    ST_SRT_OUT = SRT_EPOLL_OPT::SRT_EPOLL_OUT,
    ST_SRT_ERR = SRT_EPOLL_OPT::SRT_EPOLL_ERR,
    ST_SRT_CONN = SRT_EPOLL_OPT::SRT_EPOLL_CONNECT,
    ST_SRT_ACCEPT = SRT_EPOLL_OPT::SRT_EPOLL_ACCEPT
};

enum StSrtMode {
    ST_SRT_MODE_GLOBAL,
    ST_SRT_MODE_FILE,
    ST_SRT_MODE_LIVE,
};

// srt io info
error_t st_srt_epoll(SRTSOCKET fd, StSrtEvent evt, utime_t tm, std::string &err);

int st_srt_socket_error(SRT_SOCKSTATUS status);

int st_srt_set_global_mode(StSrtMode mode);

SRTSOCKET st_srt_create_fd(StSrtMode mode = ST_SRT_MODE_GLOBAL);

// 不推荐的方法，无法控制线程数目
error_t st_srt_connect(SRTSOCKET fd, const std::string &server,
                       int port, utime_t tm, UDPSOCKET ufd = 0,
                       bool one_or_zero_rtt = false,
                       const char *zero_req = nullptr,
                       int zero_req_len = 0);

// 推荐使用这种方法主动连接，控制客户端线程数目
error_t st_srt_connect(SRTSOCKET fd, const std::string &server, int port,
                       const std::string &self_ip, int self_port, utime_t tm,
                       bool one_or_zero_rtt = false,
                       const char *zero_req = nullptr,
                       int zero_req_len = 0);

error_t st_srt_listen(SRTSOCKET fd, const std::string &server, int port,
                      int back_log, bool reuse_port = true);

SRTSOCKET st_srt_accept(SRTSOCKET fd, struct sockaddr *addr, int *addrlen, utime_t timeout);

error_t st_srt_read(SRTSOCKET fd, void *buf, size_t nbyte, utime_t timeout, size_t &nread);

error_t st_srt_write(SRTSOCKET fd, void *buf, size_t nbyte, utime_t timeout);

error_t st_srt_close(SRTSOCKET &fd);

bool st_srt_again(int &err);

// srt stat
int st_srt_stat(SRTSOCKET fd, std::string &res, SRT_TRACEBSTATS *perf);

int st_srt_stat_print(std::string &res, SRT_TRACEBSTATS *perf);

double st_srt_stat_loss(SRT_TRACEBSTATS *perf, bool is_send);

double st_srt_stat_retrans(SRT_TRACEBSTATS *perf, bool is_send);

class SrtEventCondition {
 public:
    explicit SrtEventCondition(StSrtEvent event);

    ~SrtEventCondition();

 public:
    StSrtEvent event;
    st_cond_t cond;
    int err;
    std::string err_msg;
};

/**
* st 的各种信息回调，需要线程级别隔离
*/
class SrtStDispatch : public sps::ICoHandler {
 public:
    static SrtStDispatch *get_instance();

 public:
    SrtStDispatch();

    ~SrtStDispatch() override;

 public:
    int init();

 public:
    int st_srt_epoll(SRTSOCKET fd, StSrtEvent event, utime_t timeout, std::string &err);

 private:
    int get_exits_evt(SRTSOCKET fd);

 public:
    error_t handler() override;

 private:
    int srt_eid;
    sps::PICo co;
    std::map<SRTSOCKET, std::set<SrtEventCondition *> > conditions;
    bool stop;
};

class StSrtSocket : public IReaderWriter {
 public:
    explicit StSrtSocket(SRTSOCKET fd);

    ~StSrtSocket() override;

 public:
    // reader
    void set_recv_timeout(utime_t tm) override;

    utime_t get_recv_timeout() override;

    error_t read_fully(void *buf, size_t size, ssize_t *nread) override;

    error_t read(void *buf, size_t size, size_t &nread) override;

 public:
    // writer
    void set_send_timeout(utime_t tm) override;

    utime_t get_send_timeout() override;

    error_t write(void *buf, size_t size) override;

 public:
    SRTSOCKET get_fd();

 private:
    SRTSOCKET fd;
    utime_t rtm;
    utime_t stm;
};

}

#endif

#endif  // SPS_CO_ST_SRT_HPP_

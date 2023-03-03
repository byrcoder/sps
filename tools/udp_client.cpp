//
// Created by weideng on 2023/3/7.
//

#include <sps_st_io_udp.hpp>
#include <sps_log.hpp>
#include <sps_util_time.hpp>

union UdpPacket {
    char buf[1400];

    struct {
        int64_t seq;
        int64_t time;
    } head;
};

int main(int argc, char* argv[]) {
    using namespace sps;
    st_init();

    if (argc < 3) {
        sp_error("args ip port [rate (bps)] [gso (number)] [connect (1/0)]");
        return -1;
    }

    std::string ip       = argv[1];
    int         port     = atoi(argv[2]);
    int64_t     rate     = argc > 3 ? atoll(argv[3]) : 0;
    int64_t     inter_us = rate > 0 ? (1000 * 1000.0 / (rate/8.0/sizeof(UdpPacket))) : 0;
    int64_t     gso      = std::max(1, argc > 4 ? atoi(argv[4]) : 1);
    int         connect  = argc > 5 ? atoi(argv[5]) : 1;

    UdpPacket pkt;

    PUdpFd fd;
    UdpFd::create_fd(fd);
    StUdpClientSocket udp_client(ip, port, fd);

    int len = sizeof(pkt);
    int seq = 0;

    sp_info("->%s:%d, rate %lld, inter_us %lld, gso %lld, connect %d",
            ip.c_str(), port, rate, inter_us, gso, connect);

    if (connect && SUCCESS != udp_client.connect()) {
        sp_error("connect failed!");
        return -1;
    }

    utime_t pre       = get_time();
    utime_t now       = pre;
    int     wrote     = 0;
    int     so        = 0;
    while (true) {
        pkt.head.seq  = seq;
        pkt.head.time = now = get_time();

        if (udp_client.write(pkt.buf, len) != SUCCESS) {
            sp_error("write failed");
            break;
        }
        ++seq;
        ++wrote;
        ++so;

        if (inter_us > 0 && so >= gso) {
            st_usleep(inter_us * gso);
            so = 0;
        }

        if (now - pre >= 1000) {
            sp_info("wrote %d, %lld(ms), seq %d, rate %lld(kpbs)", wrote, now-pre, seq, (len * 8LL * wrote) / ((now-pre)));
            pre   = now;
            wrote = 0;
        }
    }

    return 0;
}
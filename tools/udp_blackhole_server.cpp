//
// Created by weideng on 2023/3/7.
//

#include <sps_sys_st_io_udp.hpp>
#include <sps_log.hpp>
#include <sps_sys_co.hpp>
#include <sps_util_time.hpp>

int main() {
    using namespace sps;
    st_init();
    auto server = std::make_shared<StUdpServerSocket>();
    error_t ret = server->listen("", 900, false, 100);

    if (ret != SUCCESS) {
        return ret;
    }

    int i = 0;
    static char buf[4096];

    while (true) {
        auto socket = server->accept();
        size_t nr = 0;

        socket->set_recv_timeout(10 * 1000 * 1000);

        time_t now  = get_time();
        time_t pre  = now;
        int64_t len = 0;
        while (socket->read(buf, sizeof(buf), nr) == SUCCESS) {
            now  = get_time();
            len += nr;
            if (now - pre >= 1000) {
                sp_info("rate %lld (kpbs)", len * 8 / (now-pre));
                pre = now;
                len = 0;
            }
        }
    }
    return 0;
}

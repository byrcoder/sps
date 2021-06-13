#include <net/sps_net_socket.hpp>

#include <st/net/sps_st_net_tcp.hpp>
#include <log/sps_log.hpp>

namespace sps {

PSocket ClientSocketFactory::create_ss(Transport transport, const std::string &ip, int port, utime_t tm) {
    error_t ret = SUCCESS;
    switch (transport) {
        case Transport::TCP: {
            st_netfd_t fd;
            if ((ret = st_tcp_connect(ip, port, tm, &fd)) != SUCCESS) {
                sp_error("Failed connect %s:%d, tm:%llu, ret:%d", ip.c_str(), port, tm, ret);
                return nullptr;
            }

            return std::make_shared<Socket>(std::make_shared<StTcpSocket>(fd), ip, port);
        }
        default:
            return nullptr;
    }
}

PIServerSocket ServerSocketFactory::create_ss(Transport transport) {
    switch (transport) {
        case Transport::TCP:
            return std::make_shared<StServerSocket>();
        default:
            return nullptr;
    }
}

}

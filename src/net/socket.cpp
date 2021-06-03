#include <net/socket.hpp>

#include <st/net/tcp.hpp>

namespace sps {

PSocket ClientSocketFactory::create_ss(Transport transport, const std::string &ip, int port, utime_t tm) {
    error_t ret = SUCCESS;
    switch (transport) {
        case Transport::TCP: {
            st_netfd_t fd;
            if ((ret = st_tcp_connect(ip, port, tm, &fd)) != SUCCESS) {
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

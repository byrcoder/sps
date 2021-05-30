#include <net/socket.hpp>
#include <http_parser.h>

#include <co/st/tcp.hpp>

namespace sps {

ServerSocketFactory& ServerSocketFactory::get_instance() {
    static ServerSocketFactory fc;
    return fc;
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

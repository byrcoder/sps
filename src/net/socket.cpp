#include <net/socket.hpp>

#include <co/st/tcp.hpp>

namespace sps {

IServerSocketFactory& IServerSocketFactory::get_instance() {
    static IServerSocketFactory fc;
    return fc;
}

PIServerSocket IServerSocketFactory::create_ss(Transport transport) {
    switch (transport) {
        case Transport::TCP:
            return std::make_shared<StServerSocket>();
        default:
            return nullptr;
    }
}

}

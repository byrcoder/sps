#ifndef SPS_CONF_SERVER_HPP
#define SPS_CONF_SERVER_HPP

#include <string>

#include <app/config/conf.hpp>

namespace sps {

struct ConfServer : public ConfigObject {
    ConfServer();

    int            listen_port;   //
    std::string    server_name;   // host
};


}

#endif //SPS_CONF_SERVER_HPP

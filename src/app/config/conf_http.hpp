#ifndef SPS_CONF_HTTP_HPP
#define SPS_CONF_HTTP_HPP

#include <app/config/conf.hpp>
#include <string>

namespace sps {

using namespace std;

struct ConfHttp : public ConfigObject {
    int       keepalive_timeout;
    string    access_log;
};

}

#endif //SPS_CONF_HTTP_HPP

#ifndef SPS_CONF_UPSTREAM_HPP
#define SPS_CONF_UPSTREAM_HPP

#include <string>

#include <app/config/conf.hpp>

namespace sps {

struct ConfUpstream : public ConfigObject {
    std::string name;
    std::string server;
    int keepalive;
};

class UpstreamConfigInstantFactory : public IConfigInstantFactory {
 public:
    PConfInstant create();
};

}

#endif  // SPS_CONF_UPSTREAM_HPP

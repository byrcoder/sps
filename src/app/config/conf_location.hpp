#ifndef SPS_CONF_LOCATION_HPP
#define SPS_CONF_LOCATION_HPP

#include <string>

#include <app/config/conf.hpp>

namespace sps {

struct ConfLocation : public ConfigObject {
    ConfLocation();

    std::string pattern;    // 匹配模式或者路径
    std::string proxy_pass; // 代理路径
};



}

#endif  // SPS_CONF_LOCATION_HPP

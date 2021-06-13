#ifndef SPS_HOST_LOC_CONFIG_HPP
#define SPS_HOST_LOC_CONFIG_HPP

#include <string>
#include <app/config/sps_config_module.hpp>

namespace sps {

// TODO: ADD add location for request

struct LocationConfCtx : public ConfCtx {
    std::string pattern;    // 匹配模式或者路径
    std::string proxy_pass; // 代理路径
};

#define OFFSET(x) offsetof(LocationConfCtx, x)
static const ConfigOption loc_options[] = {
        {"pattern",        "location name",    OFFSET(pattern),     CONF_OPT_TYPE_STRING, { .str = "/" }, },
        {"proxy_pass",     "proxy pass",       OFFSET(proxy_pass),  CONF_OPT_TYPE_STRING, { .str = "" }, },
        {nullptr }
};
#undef OFFSET

class LocationModule : public IModule {
 public:
    MODULE_CONSTRUCT(Location, loc_options);

    MODULE_CREATE_CTX(Location);
};
typedef std::shared_ptr<LocationModule> PLocationModule;

MODULE_FACTORY(Location)
class LocationModuleFactory;

}

#endif  // SPS_HOST_LOC_CONFIG_HPP

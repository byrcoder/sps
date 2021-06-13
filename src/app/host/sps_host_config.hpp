#ifndef SPS_HOST_CONFIG_HPP
#define SPS_HOST_CONFIG_HPP

#include <app/config/sps_config_module.hpp>

namespace sps {

struct HostConfCtx : public ConfCtx {
    std::string hostname;         // 域名
    std::string pass_proxy;   // 代理地址
    std::string pass_url;     // pass_url 正则
};

#define OFFSET(x) offsetof(HostConfCtx, x)
static const ConfigOption host_options[] = {
        { "hostname",  "hostname",      OFFSET(hostname),   CONF_OPT_TYPE_STRING,   {.str = "- hostname"} },
        { "pass_proxy","pass_proxy",    OFFSET(pass_proxy), CONF_OPT_TYPE_STRING,   {.str = "- pass_proxy"} },
        { "pass_url",  "pass_url",      OFFSET(pass_url),   CONF_OPT_TYPE_STRING,   {.str = "- pass_url"} },
        { "stream",    "stream module",    0,       CONF_OPT_TYPE_SUBMODULE, {.str = "-"} },
        { nullptr }
};
#undef OFFSET

class HostModule : public IModule {
 public:
    MODULE_CONSTRUCT(Host, host_options);

    MODULE_CREATE_CTX(Host);
};
typedef std::shared_ptr<HostModule> PHostModule;

MODULE_FACTORY(Host)
class HostModuleFactory;

}

#endif  // SPS_HOST_CONFIG_HPP

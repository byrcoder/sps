#ifndef SPS_HOST_CONFIG_HPP
#define SPS_HOST_CONFIG_HPP

#include <app/config/sps_config_module.hpp>
#include <app/stream/sps_stream_module.hpp>

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

 public:
    error_t post_sub_module(PIModule sub) override;

 public:
    PStreamModule stream_module;
};
typedef std::shared_ptr<HostModule> PHostModule;

MODULE_FACTORY(Host)
class HostModuleFactory;

class HostModulesRouter {
 public:
    static std::string get_wildcard_host(std::string host);

 public:
    HostModulesRouter() = default;

 public:
    error_t register_host(PHostModule host);

    PHostModule find_host(const std::string& host);

 private:
    std::map<std::string, PHostModule> hosts;
    std::map<std::string, PHostModule> exact_hosts;   // 完全匹配
    std::map<std::string, PHostModule> wildcard_hosts;    // *.匹配
    PHostModule                        default_host;  // 默认匹配
};

typedef std::shared_ptr<HostModulesRouter> PHostModulesRouter;

}

#endif  // SPS_HOST_CONFIG_HPP

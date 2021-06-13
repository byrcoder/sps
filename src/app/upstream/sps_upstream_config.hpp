#ifndef SPS_UPSTREAM_CONFIG_HPP
#define SPS_UPSTREAM_CONFIG_HPP

#include <string>
#include <map>

#include <app/config/sps_config_module.hpp>

namespace sps {

// conf value defines
struct UpStreamConfCtx : public ConfCtx {
    std::string name;
    std::string server;
    int keepalive;
};

#define OFFSET(x) offsetof(UpStreamConfCtx, x)
const ConfigOption upstream_options[] = {
        {"name",       "upstream name",    OFFSET(name),    CONF_OPT_TYPE_STRING, { .str = "unknown upstream name" }, },
        {"server",     "server array",     OFFSET(server),  CONF_OPT_TYPE_STRING, { .str = "unknown upstream server" }, },
        {"keepalive",  "timeout to keep",  OFFSET(keepalive),  CONF_OPT_TYPE_INT,    { .str = "10"  }, },
        {nullptr }
};
#undef OFFSET

// module
class UpStreamModule : public IModule {
 public:
    MODULE_CONSTRUCT(UpStream, upstream_options);

 public:
    MODULE_CREATE_CTX(UpStream);

    error_t post_conf() override;
};

typedef std::shared_ptr<UpStreamModule> PUpStreamModule;

class UpstreamModules : public Single<UpstreamModules> {
 public:
    std::map<std::string, PUpStreamModule> ups;
};

MODULE_FACTORY(UpStream)
class UpStreamModuleFactory;

}

#endif  // SPS_UPSTREAM_CONFIG_HPP

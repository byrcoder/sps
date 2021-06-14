#ifndef SPS_CORE_CONFIG_HPP
#define SPS_CORE_CONFIG_HPP

#include <app/config/sps_config_module.hpp>
#include <app/http/sps_http_module.hpp>

#include <app/upstream/sps_upstream_module.hpp>

namespace sps {

struct CoreConfCtx : public ConfCtx {
    std::string comment;
};

#define OFFSET(x) offsetof(CoreConfCtx, x)
static const ConfigOption core_options[] = {
        { "comment",  "comment", OFFSET(comment),        CONF_OPT_TYPE_STRING,    {.str = "core"} },
        { "upstream", "upstream submodule", 0,    CONF_OPT_TYPE_SUBMODULE, {.str = "upstream"} },
        { "http",     "http submodule", 0,        CONF_OPT_TYPE_SUBMODULE, {.str = "upstream"} },
        { nullptr }
};
#undef OFFSET

class CoreModule : public IModule {
 public:
    MODULE_CONSTRUCT(Core, core_options)

    MODULE_CREATE_CTX(Core);

    error_t post_sub_module(PIModule sub) override;

 public:
    std::list<PHttpModule>      http_modules;
    std::list<PUpStreamModule>  upstream_modules;
};

typedef std::shared_ptr<CoreModule> PCoreModule;

MODULE_FACTORY(Core)

}

#endif  // SPS_CORE_CONFIG_HPP

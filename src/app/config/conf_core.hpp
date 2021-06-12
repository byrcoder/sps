#ifndef SPS_CONF_CORE_HPP
#define SPS_CONF_CORE_HPP

#include <app/config/conf_module.hpp>
#include <app/http/conf_http.hpp>

#include <app/upstream/conf_upstream.hpp>

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

MODULE_FACTORY(Core)

}

#endif //SPS_CONF_CORE_HPP

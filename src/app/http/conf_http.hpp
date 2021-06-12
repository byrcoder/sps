#ifndef SPS_CONF_HTTP_HPP
#define SPS_CONF_HTTP_HPP

#include <string>

#include <app/config/conf_module.hpp>
#include <app/server/conf_server.hpp>

namespace sps {

using namespace std;

struct HttpConfCtx : public ConfCtx {
    int       keepalive_timeout;
    string    access_log;
};

#define OFFSET(x) offsetof(HttpConfCtx, x)
static const ConfigOption http_options[] = {
        {"keepalive_timeout",   "location name",    OFFSET(keepalive_timeout),     CONF_OPT_TYPE_INT,    { .str = "10" }, },
        {"access_log",          "proxy pass",       OFFSET(access_log),            CONF_OPT_TYPE_STRING, { .str = "access.log" },   },
        {"server",              "server sub module",       0,              CONF_OPT_TYPE_SUBMODULE, { .str = "" },   },

        {nullptr }
};
#undef OFFSET

class HttpModule : public IModule {
 public:
    MODULE_CONSTRUCT(Http, http_options);

    MODULE_CREATE_CTX(Http);

    error_t post_sub_module(PIModule sub) override;

 public:
    std::list<PServerModule> server_modules;
};

typedef std::shared_ptr<HttpModule> PHttpModule;

MODULE_FACTORY(Http)
class HttpModuleFactory;

}

#endif //SPS_CONF_HTTP_HPP

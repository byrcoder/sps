#ifndef SPS_CONF_SERVER_HPP
#define SPS_CONF_SERVER_HPP

#include <string>

#include <app/config/conf_module.hpp>
#include <app/config/conf_host.hpp>

namespace sps {

struct ServerConfCtx : public ConfCtx {
    int            listen_port;   // port
    std::string    server_name;   // tag for server name
    std::string    transport;     // tcp/udp/srt
};

#define OFFSET(x) offsetof(ServerConfCtx, x)
static const ConfigOption server_options[] = {
        {"listen_port",     "listen port",      OFFSET(listen_port),  CONF_OPT_TYPE_INT,      { .str = "80" }, },
        {"server_name",     "proxy pass",       OFFSET(server_name),  CONF_OPT_TYPE_STRING,   { .str = "" }, },
        {"transport",       "transport",        OFFSET(transport),    CONF_OPT_TYPE_STRING,   { .str = "tcp" }, },
        {"host",       "host sub module",       0,             CONF_OPT_TYPE_SUBMODULE,   { .str = "" }, },

        {nullptr }
};
#undef OFFSET

class ServerModule : public IModule {
 public:
    MODULE_CONSTRUCT(Server, server_options);

    MODULE_CREATE_CTX(Server);

    error_t post_sub_module(PIModule sub) override;

 public:
    std::map<std::string, PHostModule> hosts;
};

typedef std::shared_ptr<ServerModule> PServerModule ;

MODULE_FACTORY(Server)

}

#endif //SPS_CONF_SERVER_HPP

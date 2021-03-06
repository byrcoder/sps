/*****************************************************************************
MIT License
Copyright (c) 2021 byrcoder

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*****************************************************************************/

#ifndef SPS_SERVER_MODULE_HPP
#define SPS_SERVER_MODULE_HPP

#include <memory>
#include <set>
#include <string>

#include <sps_module.hpp>
#include <sps_host_module.hpp>
#include <sps_server.hpp>

namespace sps {

struct ServerConfCtx : public ConfCtx {
    int            listen_port;   // port
    std::string    server_name;   // tag for server name
    std::string    transport;     // tcp/udp/srt
    bool           reuse_port;
    int            backlog;
    utime_t        recv_timeout;
    utime_t        send_timeout;

    std::string    ssl_key_file;
    std::string    ssl_crt_file;
};

#define OFFSET(x) offsetof(ServerConfCtx, x)
static const ConfigOption server_options[] = {
        {"listen_port",     "listen port",      OFFSET(listen_port),  CONF_OPT_TYPE_INT,      { .str = "80" }, },
        {"server_name",     "proxy pass",       OFFSET(server_name),  CONF_OPT_TYPE_STRING,   { .str = "" }, },
        {"transport",       "transport",        OFFSET(transport),    CONF_OPT_TYPE_STRING,   { .str = "tcp" }, },
        {"transport",       "transport",        OFFSET(transport),    CONF_OPT_TYPE_STRING,   { .str = "tcp" }, },
        {"recv_timeout",    "recv_timeout",     OFFSET(recv_timeout), CONF_OPT_TYPE_UINT64,   { .str = "-1" }, },
        {"send_timeout",    "send_timeout",     OFFSET(send_timeout), CONF_OPT_TYPE_UINT64,   { .str = "-1" }, },

        {"reuse_port",      "reuse_port",       OFFSET(reuse_port),             CONF_OPT_TYPE_BOOL,   { .str = "off" }, },
        {"backlog",         "backlog",          OFFSET(backlog),             CONF_OPT_TYPE_INT,   { .str = "1024" }, },
        {"host",            "host sub module",       0,             CONF_OPT_TYPE_SUBMODULE,   { .str = "" }, },

        {"ssl_key_file",    "ssl key file",     OFFSET(ssl_key_file),    CONF_OPT_TYPE_STRING,   { .str = "" }, },
        {"ssl_crt_file",    "ssl crt file",     OFFSET(ssl_crt_file),    CONF_OPT_TYPE_STRING,   { .str = "" }, },

        {nullptr }
};
#undef OFFSET

class ServerCompare {
 public:
    bool operator()(const ServerConfCtx& s1, const ServerConfCtx& s2) const;
};

class ServerModule;
typedef std::shared_ptr<ServerModule> PServerModule;

class ServerModule : public IModule {
 public:
    static std::map<ServerConfCtx, PServerModule, ServerCompare> server_modules;

 public:
    static PServerModule get_server(ServerModule* sm);
    static error_t       get_router(ServerModule* sm, PHostModulesRouter &hr);

 public:
    MODULE_CONSTRUCT(Server, server_options);

    MODULE_CREATE_CTX(Server);

    error_t post_sub_module(PIModule sub) override;

    error_t merge(PIModule& module) override;

 public:
    error_t merge_global(PIModule& module);

 public:
    error_t install() override;

 public:
    error_t pre_install(PIConnHandlerFactory factory);

    error_t post_install(PHostModulesRouter router);

    error_t merge_server(PServerModule server_module);

    error_t merge_hosts(std::vector<PHostModule>& hosts);

 public:
    std::vector<PHostModule> host_modules;

 private:
    PIConnHandlerFactory socket_handler;
    PServer              server;
};

MODULE_FACTORY(Server)

}  // namespace sps

#endif  // SPS_SERVER_MODULE_HPP

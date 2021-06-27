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

#include <string>

#include <sps_module.hpp>
#include <sps_host_module.hpp>
#include "sps_server.hpp"

namespace sps {

struct ServerConfCtx : public ConfCtx {
    int            listen_port;   // port
    std::string    server_name;   // tag for server name
    std::string    transport;     // tcp/udp/srt
    bool           reuse_port;
    int            backlog;
    utime_t        recv_timeout;
    utime_t        send_timeout;
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
        {nullptr }
};
#undef OFFSET

class ServerModule : public IModule {
 public:
    MODULE_CONSTRUCT(Server, server_options);

    MODULE_CREATE_CTX(Server);

    error_t post_sub_module(PIModule sub) override;

 public:
    error_t install() override;

 public:
    error_t pre_install(PIConnHandlerFactory factory);

 public:
    PHostModulesRouter hosts_router = std::make_shared<HostModulesRouter>();
 private:
    PIConnHandlerFactory socket_handler;

 public:
    static std::vector<PServer> servers;
};
typedef std::shared_ptr<ServerModule> PServerModule ;

MODULE_FACTORY(Server)

}

#endif  // SPS_SERVER_MODULE_HPP

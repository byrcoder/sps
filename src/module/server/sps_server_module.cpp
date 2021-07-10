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

#include <sps_server_module.hpp>
#include <sps_log.hpp>

namespace sps {

error_t ServerModule::post_sub_module(PIModule sub) {
    if (sub->is_module("host")) {
        auto h = std::dynamic_pointer_cast<HostModule>(sub);
        if (!h || sub->module_name.empty()) {
            sp_error("server not found host type %s", sub->module_name.c_str());
            exit(-1);
        }

        return hosts_router->register_host(h);
    } else {
        sp_error("server not found sub module type %s",
                  sub->module_type.c_str());
        exit(-1);
    }

    return SUCCESS;
}

error_t ServerModule::install() {
    error_t ret         = SUCCESS;
    auto    server_conf = std::static_pointer_cast<sps::ServerConfCtx>(conf);
    auto    server      = std::make_shared<Server>();

    if (!socket_handler) {
        sp_error("fail install empty handler server!");
        return ERROR_CONFIG_INSTALL;
    }

    Transport t = Transport::TCP;
#ifndef SRT_DISABLED
    if (server_conf->transport == "SRT") {
        t = Transport::SRT;
    }
#endif
    ret = server->init(socket_handler, t, server_conf->send_timeout,
                        server_conf->recv_timeout);

    if (ret != SUCCESS) {
        sp_error("fail install transport %u", t);
        return ERROR_CONFIG_INSTALL;
    }

    if ((ret = server->listen("", server_conf->listen_port,
            server_conf->reuse_port, server_conf->backlog)) != SUCCESS) {
        sp_error("failed http listen port: %d, reuse: %u, backlog: %d, ret: %d",
                  server_conf->listen_port, server_conf->reuse_port,
                  server_conf->backlog, ret);

        return ret;
    }

    if ((ret = sps::ICoFactory::get_instance().start(server))!= SUCCESS) {
        sp_error("failed start server ret:%d", ret);
        return ret;
    }

    sp_info("success listen port: %d, reuse: %u, backlog: %d",
             server_conf->listen_port, server_conf->reuse_port,
             server_conf->backlog);

    servers.push_back(server);
    return ret;
}

error_t ServerModule::pre_install(PIConnHandlerFactory factory) {
    this->socket_handler = factory;
    return SUCCESS;
}

std::vector<PServer> ServerModule::servers;

}  // namespace sps

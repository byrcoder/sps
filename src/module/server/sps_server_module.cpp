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

#include <sps_host_global_module.hpp>
#include <sps_log.hpp>

namespace sps {

bool ServerCompare::operator()(const ServerConfCtx& s1, const ServerConfCtx& s2) const {
    return s1.listen_port < s2.listen_port;
}

std::map<ServerConfCtx, PServerModule, ServerCompare> ServerModule::server_modules;

PServerModule ServerModule::get_server(ServerModule *sm) {
    auto server_conf = std::static_pointer_cast<sps::ServerConfCtx>(sm->conf);
    if (server_modules.count(*server_conf)) {
        return server_modules[*server_conf];
    }
    return nullptr;
}

error_t ServerModule::get_router(ServerModule* sm, PHostModulesRouter& hr) {
    error_t ret = SUCCESS;
    hr  = std::make_shared<HostModulesRouter>();

    for (auto& h : sm->host_modules) {
        if ((ret = hr->register_host(h)) != SUCCESS) {
            sp_error("fail register host %s ret %d",
                      h->module_name.c_str(), ret);
            return ret;
        }
    }
    return ret;
}

error_t ServerModule::post_sub_module(PIModule sub) {
    if (sub->is_module("host")) {
        auto h = std::dynamic_pointer_cast<HostModule>(sub);
        if (!h || sub->module_name.empty()) {
            sp_error("server not found host type %s", sub->module_name.c_str());
            exit(-1);
        }

        host_modules.push_back(h);
        return SUCCESS;
    } else {
        sp_error("server not found sub module type %s",
                  sub->module_type.c_str());
        exit(-1);
    }

    return SUCCESS;
}

error_t ServerModule::merge(PIModule& module) {
    error_t ret = SUCCESS;

    if (!module) {
        sp_warn("merge null!");
        return ret;
    }

    if (module->is_module("globalhost")) {
        sp_info("merge globalhost");
        merge_global(module);
    }

    if (module->is_module("root")) {
        for (auto it : module->subs) {
            if (it.first != "globalhost") {
                continue;
            }

            for (auto m : it.second) {
                merge_global(m);
            }
        }
    }

    return ret;
}

error_t ServerModule::merge_global(PIModule& module) {
    if (!module->is_module("globalhost")) {
        return SUCCESS;
    }

    auto dm = dynamic_cast<GlobalHostModule*>(module.get());

    if (!dm) {
        sp_error("not global-host module!");
        exit(-1);
    }

    for (auto& hm : dm->host_modules) {
        sp_debug("server merge global host %s", hm->module_name.c_str());
        host_modules.push_back(hm);
    }

    return SUCCESS;
}

error_t ServerModule::install() {
    error_t ret         = SUCCESS;
    auto    server_conf = std::static_pointer_cast<sps::ServerConfCtx>(conf);
    server              = std::make_shared<Server>();

    if (!socket_handler) {
        sp_error("fail install empty handler server!");
        return ERROR_CONFIG_INSTALL;
    }

    Transport t = Transport::TCP;
#ifdef SRT_ENABLED
    if (server_conf->transport == "srt") {
        t = Transport::SRT;
    }
#endif

#ifdef OPENSSL_ENABLED
    if (server_conf->transport == "https") {
        t = Transport::HTTPS;
    }
#endif
    ret = server->init(socket_handler,
            server_conf->send_timeout, server_conf->recv_timeout);

    if (ret != SUCCESS) {
        sp_error("fail install transport %u", t);
        return ERROR_CONFIG_INSTALL;
    }

    if ((ret = server->listen("", server_conf->listen_port,
            server_conf->reuse_port, server_conf->backlog, t)) != SUCCESS) {
        sp_error("failed http listen port: %d, reuse: %u, backlog: %d, ret: %d",
                  server_conf->listen_port, server_conf->reuse_port,
                  server_conf->backlog, ret);

        return ret;
    }


    if ((ret = sps::ICoFactory::get_instance().start(server))!= SUCCESS) {
        sp_error("failed start server ret:%d", ret);
        return ret;
    }

    sp_info("success listen(%s) port: %d, reuse: %u, backlog: %d",
             get_transport_name(t),
             server_conf->listen_port, server_conf->reuse_port,
             server_conf->backlog);

    server_modules[*server_conf] =
            std::dynamic_pointer_cast<ServerModule>(shared_from_this());
    return ret;
}

error_t ServerModule::pre_install(PIConnHandlerFactory factory) {
    this->socket_handler = factory;
    return SUCCESS;
}

error_t ServerModule::post_install(PHostModulesRouter router) {
    error_t ret         = SUCCESS;
    auto    server_conf = std::static_pointer_cast<sps::ServerConfCtx>(conf);

    Transport t = Transport::TCP;
#ifdef SRT_ENABLED
    if (server_conf->transport == "srt") {
        t = Transport::SRT;
    }
#endif

#ifdef OPENSSL_ENABLED
    if (server_conf->transport == "https") {
        t = Transport::HTTPS;
    }
#endif

#ifdef OPENSSL_ENABLED
    if (t == Transport::HTTPS) {
        server->init_ssl(server_conf->ssl_crt_file, server_conf->ssl_key_file, router);
    }
#endif

    return ret;
}

error_t ServerModule::merge_server(PServerModule server_module) {
    return merge_hosts(server_module->host_modules);
}

error_t ServerModule::merge_hosts(std::vector<PHostModule>& hosts) {
    host_modules.insert(host_modules.end(), hosts.begin(), hosts.end());
    return SUCCESS;
}


}  // namespace sps

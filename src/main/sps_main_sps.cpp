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

#include <vector>

#include <app/core/sps_core_module.hpp>

#include <app/http/sps_http_phase_handler.hpp>
#include <app/http/sps_http_server.hpp>
#include <app/host/sps_host_router_handler.hpp>
#include <app/server/sps_server_module.hpp>
#include <app/url/sps_url_protocol.hpp>

#include <install/sps_install.hpp>

#include <co/sps_co.hpp>

#include <log/sps_log.hpp>

#include <sync/sps_sync.hpp>


#include <sps_typedef.hpp>


std::vector<sps::PServer> servers;
sps::PCoreModule core_module;

error_t init_co() {
    return sps::ICoFactory::get_instance().init();
}

error_t init_config(const char* filename) {
    error_t ret           = SUCCESS;
    auto&   protocols     = SingleInstance<sps::UrlProtocol>::get_instance();

    auto    file_reader   = protocols.create("file://" + std::string(filename));

    if (!file_reader) {
        sp_error("url-file-protocol not found");
        return ERROR_URL_PROTOCOL_NOT_EXISTS;
    }

    core_module = std::dynamic_pointer_cast<sps::CoreModule>(SingleInstance<sps::ModuleFactoryRegister>::get_instance()
            .create("core", "",  nullptr));

    if ((ret = file_reader->open(std::string(filename))) != SUCCESS) {
        sp_error("Failed open file %s", filename);
        return ret;
    }

    if ((ret = core_module->init_conf(file_reader)) != SUCCESS) {
        sp_error("Failed init config(%s), ret:%d", filename, ret);
        return ret;
    }

    return SUCCESS;
}

error_t run_http_server() {
    error_t ret = SUCCESS;
    for (auto& http : core_module->http_modules) {
        for (auto& server : http->server_modules) {
            auto server_conf = std::static_pointer_cast<sps::ServerConfCtx>(server->conf);
            auto hp          = std::make_shared<sps::HttpPhaseHandler>();

            hp->reg(std::make_shared<sps::HttpParsePhaseHandler>());       // parser
            hp->reg(std::make_shared<sps::HostRouterPhaseHandler>(
                    server->hosts_router,
                    std::make_shared<sps::HttpProxyPhaseHandler>(),
                    SingleInstance<sps::Http404PhaseHandler>::get_instance_share_ptr())); // router

            auto http_server = std::make_shared<sps::HttpServer>(hp);

            if (http_server->listen("", server_conf->listen_port) != SUCCESS) {
                sp_error("Failed http listen :%d", server_conf->listen_port);
                return 0;
            }
            sp_info("success http server listen %d", server_conf->listen_port);
            servers.push_back(http_server);

            ret = sps::ICoFactory::get_instance().start(http_server);
            if (ret != SUCCESS) {
                sp_error("Failed start http server ret:%d", ret);
                return ret;
            }
        }
    }

    return ret;
}

error_t run_servers() {
    error_t ret = SUCCESS;

    if ((ret = run_http_server()) != SUCCESS) {
        sp_error("Failed start http server ret:%d", ret);
        return ret;
    }

    return ret;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        sp_error("Argv must greater 1: ./build/sps conf/sps.conf");
        return -1;
    }

    sps::sps_once_install();

    auto ret = init_config(argv[1]);

    if (ret != SUCCESS) {
        return ret;
    }

    if ((ret = init_co()) != SUCCESS) {
        sp_error("Failed co init ret:%d", ret);
        return ret;
    }

    if ((ret = run_servers()) != SUCCESS) {
        return ret;
    }

    SingleInstance<sps::Sleep>::get_instance().sleep(SLEEP_FOREVER);
    return 0;
}
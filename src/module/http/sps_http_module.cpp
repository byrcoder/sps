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

#include <sps_http_module.hpp>
#include <sps_log.hpp>
#include <module/host/sps_host_router_handler.hpp>
#include "sps_http_server.hpp"
#include "sps_http_adapter_phase_handler.hpp"

namespace sps {

error_t HttpModule::post_sub_module(PIModule sub) {
    if (sub->module_type == "server") {
        auto s = dynamic_pointer_cast<ServerModule>(sub);
        if (!s) {
            sp_error("http sub module not server %s", sub->module_type.c_str());
            exit(-1);
        }

        server_modules.push_back(s);
    } else {
        sp_error("http sub module %s not found", sub->module_type.c_str());
        exit(-1);
    }

    return SUCCESS;
}

error_t HttpModule::install() {
    error_t ret = SUCCESS;

    for (auto& s : server_modules) {
        auto handler     = std::make_shared<sps::HttpPhaseHandler>();

        // http header parser
        handler->reg(std::make_shared<sps::HttpParsePhaseHandler>());

        // host router -> do host handler or default return 404
        handler->reg(std::make_shared<sps::HostRouterPhaseHandler>(
                s->hosts_router,
                std::make_shared<sps::HttpAdapterPhaseHandler>(),
                SingleInstance<sps::Http404PhaseHandler>::get_instance_share_ptr()));

        s->pre_install(std::make_shared<HttpConnectionHandlerFactory>(handler));

        if ((ret = s->install()) != SUCCESS) {
            sp_error("failed install %s http server", s->module_name.c_str());
            return ret;
        }
        sp_info("success install %s http server listen", s->module_name.c_str());
    }
    return ret;
}

}

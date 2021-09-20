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

//
// Created by byrcoder on 2021/8/24.
//

#include <sps_srt_module.hpp>
#include <sps_host_phase_handler.hpp>
#include <sps_host_router_handler.hpp>
#include <sps_srt_server.hpp>
#include <sps_srt_server_handler.hpp>
#include <sps_srt_stream_handler.hpp>

#ifdef SRT_ENABLED

namespace sps {

error_t SrtModule::post_sub_module(PIModule sub) {
    if (sub->is_module("server")) {
        auto s = dynamic_pointer_cast<ServerModule>(sub);
        if (!s) {
            sp_error("http sub module not server %s", sub->module_type.c_str());
            exit(-1);
        }

        srt_servers.push_back(s);
    } else {
        sp_error("http sub module %s not found", sub->module_type.c_str());
        exit(-1);
    }

    return SUCCESS;
}

error_t SrtModule::merge(PIModule& module) {
    error_t ret = SUCCESS;

    sp_info("srt server merge %s %lu", module->module_type.c_str(),
            srt_servers.size());
    for (auto& rs : srt_servers) {
        if ((ret = rs->merge(module)) != SUCCESS) {
            sp_error("rtmp server merge fail ret %d", ret);
            return ret;
        }
    }
    return ret;
}

error_t SrtModule::install() {
    error_t ret = SUCCESS;

    for (auto& s : srt_servers) {
        PServerModule ps = ServerModule::get_server(s.get());
        PHostModulesRouter hr;

        if (ps) {
            if ((ret = ps->merge_server(s)) != SUCCESS) {
                sp_error("fail merge ret %d", ret);
                return ret;
            }
            continue;
        }

        if ((ret = ServerModule::get_router(s.get(), hr))
            != SUCCESS) {
            sp_error("fail get router ret %d", ret);
            return ret;
        }

        auto srt_handler = std::make_shared<ServerPhaseHandler>();

        // srt streamid parse
        srt_handler->reg(std::make_shared<SrtPrepareHandler>());
        // srt host router and publishing or play
        srt_handler->reg(std::make_shared<HostRouterPhaseHandler>(
                hr, std::make_shared<SrtServerStreamHandler>(),
                nullptr));

        s->pre_install(std::make_shared<SrtConnHandlerFactory>(srt_handler));

        if ((ret = s->install()) != SUCCESS) {
            sp_error("failed install %s rtmp server", s->module_name.c_str());
            return ret;
        }
        sp_info("success install %s srt server listen",
                s->module_name.c_str());
    }
    return ret;
}

}

#endif

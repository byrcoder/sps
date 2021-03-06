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
// Created by byrcoder on 2021/7/14.
//

#include <sps_host_global_module.hpp>

namespace sps {

std::vector<PGlobalHostModule> GlobalHostModule::global_hosts;

error_t GlobalHostModule::post_sub_module(PIModule sub) {
    if (sub->is_module("host")) {
        auto h = std::dynamic_pointer_cast<HostModule>(sub);
        if (!h || sub->module_name.empty()) {
            sp_error("server not found host type %s", sub->module_name.c_str());
            exit(-1);
        }

        host_modules.push_back(h);
        return SUCCESS;
    } else {
        sp_error("global host not found sub module type %s",
                 sub->module_type.c_str());
        exit(-1);
    }

    return SUCCESS;
}

error_t GlobalHostModule::post_conf() {
    global_hosts.push_back(std::dynamic_pointer_cast<GlobalHostModule>(
            shared_from_this()));
    return SUCCESS;
}


}

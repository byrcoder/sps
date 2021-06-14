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

#include <app/core/sps_core_module.hpp>

#include <log/sps_log.hpp>

namespace sps {

error_t CoreModule::post_sub_module(PIModule sub) {
    if (sub->module_type == "http") {
        auto hm = std::dynamic_pointer_cast<HttpModule>(sub);
        if (!hm) {
            sp_error("core sub module not http %s,%s. %s",
                    sub->module_type.c_str(), sub->module_name.c_str(), typeid(sub.get()).name());
            return ERROR_MODULE_TYPE_NOT_MATCH;
        }
        http_modules.push_back(hm);
    } else if (sub->module_type == "upstream") {
        auto up = std::dynamic_pointer_cast<UpStreamModule>(sub);
        if (!up) {
            sp_error("core sub module not upstream %s,%s.", sub->module_type.c_str(), sub->module_name.c_str());
            return ERROR_MODULE_TYPE_NOT_MATCH;
        }
        upstream_modules.push_back(up);
    } else {
        sp_error("core not found sub module type %s", sub->module_type.c_str());
        return ERROR_MODULE_TYPE_NOT_MATCH;
    }

    return SUCCESS;
}

}
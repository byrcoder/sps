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

#include <sps_host_module.hpp>

namespace sps {

error_t HostModule::post_sub_module(PIModule sub) {
    if (sub->is_module("stream")) {
        stream_module = std::dynamic_pointer_cast<StreamModule>(sub);

        if (!stream_module) {
            sp_error("http sub module not stream %s,%s. %s",
                     sub->module_type.c_str(),
                     sub->module_name.c_str(),
                     typeid(sub.get()).name());
            return ERROR_MODULE_TYPE_NOT_MATCH;
        }

        return SUCCESS;
    } else {
        sp_error("http not found sub module type %s", sub->module_type.c_str());
        return ERROR_MODULE_TYPE_NOT_MATCH;
    }
}

std::string HostModulesRouter::get_wildcard_host(std::string host) {
    // host *.github.com; -> moc.buhtig

    auto n = host.find('.');
    if (n == std::string::npos) {
        return "";
    }

    host = host.substr(n);
    std::reverse(host.begin(), host.end());

    return host;
}

error_t HostModulesRouter::register_host(PHostModule host) {
    if (host->module_name[0] != '*') {
        // full match
        exact_hosts[host->module_name] = std::move(host);
        return SUCCESS;
    } else if (host->module_name[1] == '\0') {
        // default
        default_host = host;
    } else if (host->module_name[1] != '.' || host->module_name[2] == '\0') {
        // illegal match
        sp_error("Unsupported Host start with %s", host->module_name.c_str());
        return ERROR_CONFIG_PARSE_INVALID;
    }
    // else
    // host *.github.com; -> moc.buhtig
    std::string wildcard_host = get_wildcard_host(host->module_name);
    wildcard_hosts[wildcard_host] = host;
    return SUCCESS;
}

PHostModule HostModulesRouter::find_host(const std::string &host) {
    auto it = exact_hosts.find(host);
    if (it != exact_hosts.end()) {
        return it->second;
    }

    if (wildcard_hosts.empty()) {
        return default_host;
    }

    auto wild_host = get_wildcard_host(host);
    it = wildcard_hosts.find(wild_host);
    if (it != wildcard_hosts.end()) {
        return it->second;
    }

    return default_host;
}

}  // namespace sps

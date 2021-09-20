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

const char* krole_proxy = "proxy";
const char* krole_source = "source";

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

bool HostModule::enabled() {
    auto host = static_cast<HostConfCtx*>(conf.get());
    return host->enabled != 0;
}

bool HostModule::publish() {
    auto host = static_cast<HostConfCtx*>(conf.get());
    return host->role == krole_source;
}

bool HostModule::proxy() {
    auto host = static_cast<HostConfCtx*>(conf.get());
    return host->role == krole_proxy;
}

bool HostModule::is_streaming() {
    auto host = static_cast<HostConfCtx*>(conf.get());
    return host->streaming != 0;
}

bool HostModule::support_publish(const std::string& format) {
    auto host = static_cast<HostConfCtx*>(conf.get());

    return host->edge_avformat == "all" || host->edge_avformat == format;
}

std::string HostModule::edge_format() {
    auto host = static_cast<HostConfCtx*>(conf.get());
    return host->edge_avformat;
}

std::string HostModule::pass_proxy() {
    auto host = static_cast<HostConfCtx*>(conf.get());
    return host->pass_proxy;
}

std::string HostModule::role() {
    auto host = static_cast<HostConfCtx*>(conf.get());
    return host->role;
}

std::string HostModule::ssl_key_file() {
    auto host = static_cast<HostConfCtx*>(conf.get());
    return host->ssl_key_file;
}

std::string HostModule::ssl_cert_file() {
    auto host = static_cast<HostConfCtx*>(conf.get());
    return host->ssl_crt_file;
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
        if (exact_hosts.count(host->module_name)) {
            sp_error("dup host %s", host->module_name.c_str());
            return ERROR_CONFIG_PARSE_INVALID;
        }

        // full match
        exact_hosts[host->module_name] = std::move(host);
        return SUCCESS;
    } else if (host->module_name[1] == '\0') {
        if (default_host) {
            sp_error("dup default %s", host->module_name.c_str());
            return ERROR_CONFIG_PARSE_INVALID;
        }

        // * default
        default_host = host;
    } else if (host->module_name[1] != '.' || host->module_name[2] == '\0') {
        // illegal match
        sp_error("Unsupported Host start with %s", host->module_name.c_str());
        return ERROR_CONFIG_PARSE_INVALID;
    } else {
        // host *.github.com; -> moc.buhtig
        std::string wildcard_host = get_wildcard_host(host->module_name);

        if (wildcard_hosts.count(wildcard_host)) {
            sp_error("dup wildcard host %s", wildcard_host.c_str());
            return ERROR_CONFIG_PARSE_INVALID;
        }
        wildcard_hosts[wildcard_host] = host;
    }
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

error_t HostModulesRouter::merge(HostModulesRouter *router) {
    error_t ret = SUCCESS;
    if (router->default_host && default_host) {
        sp_error("fail host merge dup default");
        return ERROR_CONFIG_PARSE_INVALID;
    }

    for (auto& it : router->exact_hosts) {
        if (exact_hosts.count(it.first)) {
            sp_error("fail host merge dup exact %s", it.first.c_str());
            return ERROR_CONFIG_PARSE_INVALID;
        }
        exact_hosts[it.first] = it.second;
    }

    for (auto& it : router->wildcard_hosts) {
        if (wildcard_hosts.count(it.first)) {
            sp_error("fail host merge dup wildcard %s", it.first.c_str());
            return ERROR_CONFIG_PARSE_INVALID;
        }
        wildcard_hosts[it.first] = it.second;
    }

    return SUCCESS;
}

#ifdef OPENSSL_ENABLED
error_t HostModulesRouter::search(const std::string& server_name, SSLConfig& config) {
    auto host = find_host(server_name);
    if (!host) {
        sp_warn("ssl not found %s", server_name.c_str());
        return ERROR_SSL_CERT_NOT_FOUND;
    }

    config.key_file = host->ssl_key_file();
    config.crt_file = host->ssl_cert_file();

    if (config.key_file.empty() || config.crt_file.empty()) {
        sp_warn("ssl not config %s", server_name.c_str());
        return ERROR_SSL_CERT_NOT_FOUND;
    }

    return SUCCESS;
}
#endif

}  // namespace sps

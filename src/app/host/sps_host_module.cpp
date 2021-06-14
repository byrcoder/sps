#include <app/host/sps_host_module.hpp>

namespace sps {

error_t HostModule::post_sub_module(PIModule sub) {
    if (sub->module_type == "stream") {
        auto hm = std::dynamic_pointer_cast<StreamModule>(sub);
        if (!hm) {
            sp_error("http sub module not stream %s,%s. %s",
                     sub->module_type.c_str(), sub->module_name.c_str(), typeid(sub.get()).name());
            return ERROR_MODULE_TYPE_NOT_MATCH;
        }
        stream_module = std::move(hm);
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
    if (host->module_name[0] != '*') {        // 完全匹配
        exact_hosts[host->module_name] = std::move(host);
        return SUCCESS;
    } else if(host->module_name[1] == '\0') { // 默认匹配
        default_host = host;
    } else if (host->module_name[1] != '.' || host->module_name[2] == '\0') { // 非法格式
        sp_error("Unsupported Host start with %s", host->module_name.c_str());
        return ERROR_CONFIG_PARSE_INVALID;
    }
    // else
    // host *.github.com; -> moc.buhtig
    std::string wildcard_host = get_wildcard_host(host->module_name);
    wildcard_hosts[wildcard_host] = host;
    return SUCCESS;
}

PHostModule HostModulesRouter::find_host(const std::string& host) {
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

}
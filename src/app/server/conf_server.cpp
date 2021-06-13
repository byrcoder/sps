#include <app/server/conf_server.hpp>
#include <log/log_logger.hpp>

namespace sps {

error_t ServerModule::post_sub_module(PIModule sub) {
    if (sub->module_type == "host") {
        auto h = std::dynamic_pointer_cast<HostModule>(sub);
        if (!h || sub->module_name.empty()) {
            sp_error("server not found host type %s", sub->module_name.c_str());
            exit(-1);
        }

        hosts[sub->module_name] = h;
        return add_router(h);
    } else {
        sp_error("server not found sub module type %s", sub->module_type.c_str());
        exit(-1);
    }

    return SUCCESS;
}

error_t ServerModule::add_router(PHostModule host) {
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

std::string ServerModule::get_wildcard_host(std::string host) {
    // host *.github.com; -> moc.buhtig

    auto n = host.find('.');
    if (n == std::string::npos) {
        return "";
    }

    host = host.substr(n);
    std::reverse(host.begin(), host.end());
    return host;
}

}

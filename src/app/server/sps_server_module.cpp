#include <app/server/sps_server_module.hpp>
#include <log/sps_log.hpp>

namespace sps {

error_t ServerModule::post_sub_module(PIModule sub) {
    if (sub->module_type == "host") {
        auto h = std::dynamic_pointer_cast<HostModule>(sub);
        if (!h || sub->module_name.empty()) {
            sp_error("server not found host type %s", sub->module_name.c_str());
            exit(-1);
        }

        return hosts_router->register_host(h);
    } else {
        sp_error("server not found sub module type %s", sub->module_type.c_str());
        exit(-1);
    }

    return SUCCESS;
}

}

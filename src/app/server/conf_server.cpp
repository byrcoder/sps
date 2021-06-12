#include <app/server/conf_server.hpp>
#include <log/log_logger.hpp>

namespace sps {

error_t ServerModule::post_sub_module(PIModule sub) {
    if (sub->module_type == "host") {
        auto h = std::dynamic_pointer_cast<HostModule>(sub);
        if (!h) {
            sp_error("server not found host type %s", sub->module_name.c_str());
            exit(-1);
        }
        hosts[sub->module_name] = h;
    } else {
        sp_error("server not found sub module type %s", sub->module_type.c_str());
        exit(-1);
    }

    return SUCCESS;
}

}

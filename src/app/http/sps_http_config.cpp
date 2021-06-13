#include <app/http/sps_http_config.hpp>
#include <log/sps_log.hpp>

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

}

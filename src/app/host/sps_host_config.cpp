#include <app/host/sps_host_config.hpp>

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

}
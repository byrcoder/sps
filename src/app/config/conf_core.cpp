#include <app/config/conf_core.hpp>

#include <log/log_logger.hpp>

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
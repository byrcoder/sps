#include <app/upstream/conf_upstream.hpp>
#include <memory>

namespace sps {

error_t UpStreamModule::post_conf() {
    auto& obj = SingleInstance<UpstreamModules>::get_instance();
    obj.ups[this->module_name] = std::dynamic_pointer_cast<UpStreamModule>(shared_from_this());
    return SUCCESS;
}

}
#include <app/config/conf.hpp>
#include <utility>
#include <log/log_logger.hpp>

namespace sps {

ConfigItem::ConfigItem(std::string name, std::string args):
    name(std::move(name)), args(std::move(args)) {

}

error_t ConfigOption::opt_set(void *obj, const char *val) {
    if (!obj) {
        sp_error("set opt empty");
    }

    return SUCCESS;
}


}
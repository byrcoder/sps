#include <app/config/conf.hpp>

namespace sps {

ConfigItem::ConfigItem(const std::string &name, std::vector<std::string> &args):
    name(name), args(args) {

}

}
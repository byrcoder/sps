#ifndef SPS_CONF_HPP
#define SPS_CONF_HPP

#include <memory>
#include <string>
#include <vector>

namespace sps {

/*
 * config设置的层级结构
 * config --- global
 *        --- module(http) --- global
 *                         --- server --- global
 *                                    --- host --- global
 *                                             --- location
 *
 *
 */
class ConfigItem {
 public:
    ConfigItem(const std::string& name, std::vector<std::string>& args);

 public:
    std::string name;
    std::vector<std::string> args;
};
typedef std::shared_ptr<ConfigItem> PConfigItem;

class Config;
typedef std::shared_ptr<Config> PConfig ;

// Config需要从固态化，转成实际的配置，提升服务性能
class Config {
 public:
    Config() = default;

 public:
    std::vector<PConfigItem> items;
    std::vector<PConfig>     submodules; // submodule 可以继承上层items的特性
};

class ConfigInstance {

};

}

#endif // SPS_CONF_HPP

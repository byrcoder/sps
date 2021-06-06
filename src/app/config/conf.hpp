#ifndef SPS_CONF_HPP
#define SPS_CONF_HPP

#include <map>
#include <memory>
#include <string>
#include <vector>
#include <typedef.hpp>

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
    ConfigItem(std::string  name, std::string  value);

 public:
    std::string name;
    std::string args;
};
typedef std::shared_ptr<ConfigItem> PConfigItem;

class Config;
typedef std::shared_ptr<Config> PConfig ;

// Config需要从固态化，转成实际的配置，提升服务性能
class Config {
 public:
    Config() = default;

 public:
    PConfigItem get_item(const char* name);

 public:
    std::string              name;       // 树节点的名字，根目录为空，其他的如http server tls
    std::vector<PConfigItem> items;      // 树和子节点的名字
    std::vector<PConfig>     submodules; // submodule 可以继承上层items的特性
};

enum ConfigOptionType {
    CONF_OPT_TYPE_INT,
    CONF_OPT_TYPE_INT64,
    CONF_OPT_TYPE_DOUBLE,
    CONF_OPT_TYPE_FLOAT,

    CONF_OPT_TYPE_CSTRING, // c string
    CONF_OPT_TYPE_STRING,  // c++ string
    CONF_OPT_TYPE_UINT64,
    CONF_OPT_TYPE_BOOL,

    CONF_OPT_TYPE_SUBMODULE     // submodules
};

// work as AVOption
struct ConfigOption {
    const char *name;

    /**
     * show help
     */
    const char *help;

    /**
     * value offset the object
     */
    int offset;
    enum ConfigOptionType type;

    /**
     * the default value for scalar options
     */
    union {
        int64_t i64;
        double dbl;
        const char *str;
    } default_val;

    error_t opt_set(void* obj, const char* val) const;
};

// 实例化
class IConfigInstant {
 public:
    virtual error_t set_opts(const ConfigOption* opts, PConfig value);
};

typedef std::shared_ptr<IConfigInstant> PConfInstant ;

class IConfigInstantFactory {
 public:
    virtual ~IConfigInstantFactory() = default;
    virtual PConfInstant create() = 0;
};

typedef std::shared_ptr<IConfigInstantFactory> PConfigFactory ;

class ConfigFactoryRegister {
 public:
    error_t reg(const std::string& name, PConfigFactory factory);
    PConfInstant create(const std::string& name);

 private:
    std::map<std::string, PConfigFactory> factories;
};

}

#endif // SPS_CONF_HPP

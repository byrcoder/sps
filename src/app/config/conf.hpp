#ifndef SPS_CONF_HPP
#define SPS_CONF_HPP

#include <map>
#include <memory>
#include <string>
#include <vector>
#include <typedef.hpp>
#include <net/net_io.hpp>

namespace sps {

enum LineType {
    ROOT_START,

    ITEM,
    SUBMODULE_START,
    SUBMODULE_END,

    ROOT_END,
};

class FileReader : public IReader {
 public:
    error_t read_line(const std::string& name, const std::string arg, LineType& type);
};

typedef std::shared_ptr<FileReader> PFileReader;

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

class ConfigInstant;
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
        const char *str;
    } default_val;

    error_t opt_set(void* obj, const char* val, int len) const;
};

class ConfigInstant;
typedef std::shared_ptr<ConfigInstant> PConfInstant;

struct ConfigObject {
};

typedef std::shared_ptr<ConfigObject> PConfigObject;

// 实例化
class ConfigInstant {
 public:
    ConfigInstant(const char* name, PConfigObject obj,
                  const ConfigOption opts[],
                  bool is_root = false);

 public:
    virtual error_t set_default();
    virtual error_t set_opts(PIReader rd, int &line);
    virtual error_t parse(const std::string& line, std::string& cmd, std::string& arg, LineType& lt);

 public:
    const char*         name;
    PConfigObject       obj;
    const ConfigOption* opts;
    bool                is_root;

};

class IConfigInstantFactory {
 public:
    virtual ~IConfigInstantFactory() = default;
    virtual PConfInstant create() = 0;
};

typedef std::unique_ptr<IConfigInstantFactory> PConfigFactory ;

class ConfigFactoryRegister {
 public:
    error_t reg(const std::string& name, PConfigFactory factory);
    PConfInstant create(const std::string& name);

 private:
    std::map<std::string, PConfigFactory> factories;
};

}

#endif // SPS_CONF_HPP

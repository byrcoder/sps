#ifndef SPS_CONFIG_OPT_HPP
#define SPS_CONFIG_OPT_HPP

#include <map>
#include <memory>
#include <string>
#include <vector>
#include <sps_typedef.hpp>
#include <net/sps_net_io.hpp>

namespace sps {

enum LineType {
    ROOT_START,

    EMPTY,
    COMMENT,

    ITEM,
    SUBMODULE_START,
    SUBMODULE_END,

    ROOT_END,
};

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

}

#endif  // SPS_CONFIG_OPT_HPP

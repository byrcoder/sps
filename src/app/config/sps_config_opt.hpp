/*****************************************************************************
MIT License
Copyright (c) 2021 byrcoder

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*****************************************************************************/

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

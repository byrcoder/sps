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

#include <sps_module_opt.hpp>

#include <cstdlib>
#include <utility>

#include <log/sps_log.hpp>

namespace sps {

error_t ConfigOption::opt_set(void *obj, const char *val, int len) const {
    if (!obj) {
        sp_error("set opt empty");
        return ERROR_CONFIG_OPT_SET;
    }

    uint8_t *dst = ((uint8_t *) obj) + offset;

    switch (type) {
        case CONF_OPT_TYPE_INT: {
            int num = len > 0 ? atoi(val) : 0;
            memcpy(dst, (void*) &num, sizeof(num));

            sp_debug("set int { %s->%d } success", name, num);

            break;
        }
        case CONF_OPT_TYPE_INT64:
        case CONF_OPT_TYPE_UINT64: {
            int64_t num = len > 0 ? atoll(val) : 0;
            memcpy(dst, (void*) &num, sizeof(num));

            sp_debug("set uint64 { %s->%lld } success", name, num);

            break;
        }
        case CONF_OPT_TYPE_DOUBLE: {
            double num = len > 0 ? atof(val) : 0;
            memcpy(dst, (void*) &num, sizeof(num));

            sp_debug("set double { %s->%f } success", name, num);

            break;
        }
        case CONF_OPT_TYPE_FLOAT: {
            float num = len > 0 ? atof(val) : 0;
            memcpy(dst, (void*) &num, sizeof(num));

            sp_debug("set float { %s->%f } success", name, num);

            break;
        }
        case CONF_OPT_TYPE_CSTRING: {
            size_t len = strlen(val) + 1;
            *((char **) dst) = new char[len];
            memcpy(*(char **) dst, val, len);

            // sp_debug("set cstring { %s->%s } success", name, val);
        }
            break;
        case CONF_OPT_TYPE_STRING:
            *((std::string*) dst) = val;

            sp_debug("set string { %s->%s } success", name, val);
            break;
        case CONF_OPT_TYPE_BOOL: {
            bool num = memcmp(val, "on", 2) == 0;
            memcpy(dst, (void*) &num, sizeof(num));

            sp_debug("------------------bool val %s -> %d-----------",
                     val, num);
            break;
        }
        case CONF_OPT_TYPE_SUBMODULE:

        default:
            return ERROR_CONFIG_OPT_TYPE;
    }

    return SUCCESS;
}

}  // namespace sps

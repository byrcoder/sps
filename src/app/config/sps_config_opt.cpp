#include <app/config/sps_config_opt.hpp>

#include <cstdlib>
#include <utility>
#include <sstream>

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
        case CONF_OPT_TYPE_FLOAT:{
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
            bool num = memcmp(val, "on", 3) == 0;
            memcpy(dst, val, sizeof(num));
            break;
        }
        case CONF_OPT_TYPE_SUBMODULE:

        default:
            return ERROR_CONFIG_OPT_TYPE;
    }

    return SUCCESS;
}



}
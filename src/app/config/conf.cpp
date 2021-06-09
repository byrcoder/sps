#include <app/config/conf.hpp>
#include <utility>
#include <log/log_logger.hpp>

namespace sps {

error_t FileReader::read_line(const std::string &name, const std::string arg, LineType &type) {
    return SUCCESS;
}

error_t ConfigOption::opt_set(void *obj, const char *val) const {
    if (!obj) {
        sp_error("set opt empty");
        return ERROR_CONFIG_OPT_SET;
    }

    uint8_t *dst = ((uint8_t *) obj) + offset;

    switch (type) {
        case CONF_OPT_TYPE_INT:
            memcpy(dst, val, sizeof(int));
            break;
        case CONF_OPT_TYPE_INT64:
        case CONF_OPT_TYPE_UINT64:
            memcpy(dst, val, sizeof(int64_t));
            break;
        case CONF_OPT_TYPE_DOUBLE:
            memcpy(dst, val, sizeof(double));
            break;
        case CONF_OPT_TYPE_FLOAT:
            memcpy(dst, val, sizeof(float));
            break;
        case CONF_OPT_TYPE_CSTRING: {
            size_t len = strlen(val) + 1;
            *((char **) dst) = new char[len];
            memcpy(*(char **) dst, val, len);
        }
            break;
        case CONF_OPT_TYPE_STRING:
            *((std::string*) dst) = val;
            break;
        case CONF_OPT_TYPE_BOOL:
            memcpy(dst, val, sizeof(bool));
            break;
        case CONF_OPT_TYPE_SUBMODULE:

        default:
            return ERROR_CONFIG_OPT_TYPE;
    }

    return SUCCESS;
}

ConfigInstant::ConfigInstant(PConfigObject obj, const ConfigOption* opts, bool is_root) {
    this->is_root = is_root;
    this->opts    = opts;
    this->obj     = std::move(obj);
}

error_t ConfigInstant::set_default() {
    error_t ret = SUCCESS;

    for (int i = 0;  (opts[i].name); ++i) {
        auto  opt        =  opts+i;
        const char* val  =  opt->default_val.str;
        if ((ret = opt->opt_set(obj.get(), val)) != SUCCESS) {
            sp_error("failed set opt ret:%d", ret);
            return ret;
        }
    }

    return ret;
}

error_t ConfigInstant::set_opts(PIReader rd, int &line) {
    error_t ret = SUCCESS;

    std::string strline;
    std::string name;
    std::string arg;
    LineType    line_type = is_root ? ROOT_START : SUBMODULE_START; // 模块刚启动

    if ((ret = set_default()) != SUCCESS) {
        return ret;
    }

    while ((ret = rd->read_line(strline)) == SUCCESS && (ret = parse(strline, name, arg, line_type)) == SUCCESS) {
        ++line;
        int i = 0;

        if (line_type == SUBMODULE_END || line_type == ROOT_END) {
            break;
        }

        while (opts[i].name && name != opts[i].name) ++i;

        if (!opts[i].name) {
            sp_error("skip line:%d, %s %s", line, name.c_str(), arg.c_str());
            continue;
        }
        switch (line_type) {
            case ITEM:
                if (opts[i].type == CONF_OPT_TYPE_SUBMODULE) {
                    sp_error("item expect line:%d, %s %s", line, name.c_str(), arg.c_str());
                    return ERROR_CONFIG_PARSE_INVALID;
                }
                opts[i].opt_set(obj.get(), arg.c_str());
                break;
            case SUBMODULE_START:
                if (opts[i].type != CONF_OPT_TYPE_SUBMODULE) {
                    sp_error("submodule expect line:%d, %s %s", line, name.c_str(), arg.c_str());
                    return ERROR_CONFIG_PARSE_INVALID;
                }
                break;
            default:
                sp_error("submodule expect line:%d, %s %s", line, name.c_str(), arg.c_str());
                exit(-1); // illegal be here
        }
    }


    if (ret != SUCCESS) {
        return ret;
    }

    if ((is_root && line_type == ROOT_END) || (!is_root && line_type == SUBMODULE_END)) {
        return SUCCESS;
    }

    sp_error("unexpected end line:%d, %s %s", line, name.c_str(), arg.c_str());
    return ERROR_CONFIG_PARSE_INVALID;
}

error_t ConfigInstant::parse(const std::string &line, std::string &name, std::string &arg, LineType &lt) {
    return SUCCESS;
}

error_t ConfigFactoryRegister::reg(const std::string &name, PConfigFactory factory) {
    if (factories.count(name)) {
        sp_error("dup factory name:%s", name.c_str());
        return ERROR_CONFIG_FACTORY_DUP;
    }

    factories[name] = std::move(factory);
    return SUCCESS;
}

PConfInstant ConfigFactoryRegister::create(const std::string &name) {
    auto it = factories.find(name);

    if (it == factories.end()) {
        return nullptr;
    }

    return it->second->create();
}

}
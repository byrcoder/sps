#include <app/config/conf.hpp>
#include <utility>
#include <log/log_logger.hpp>

namespace sps {

ConfigItem::ConfigItem(std::string name, std::string args):
    name(std::move(name)), args(std::move(args)) {
}

PConfigItem Config::get_item(const char *name) {
    for (auto& item : items) {
        if (item->name == name) return item;
    }
    return nullptr;
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

error_t IConfigInstant::set_opts(const ConfigOption *opts,  PConfig value) {
    error_t ret = SUCCESS;
    void*   obj = (void*) this;

    for (int i = 0;  (opts[i].name); ++i) {
        auto  opt        = opts+i;
        auto  conf       = value->get_item(opt->name);
        const char* val  = conf ? conf->args.c_str() : opt->default_val.str;

        if ((ret = opt->opt_set(obj, val)) != SUCCESS) {
            sp_error("failed set opt ret:%d", ret);
            return ret;
        }
    }

    return ret;
}

error_t ConfigFactoryRegister::reg(const std::string &name, PConfigFactory factory) {
    if (factories.count(name)) {
        sp_error("dup factory name:%s", name.c_str());
        return ERROR_CONFIG_FACTORY_DUP;
    }

    factories[name] = factory;
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
#include <app/config/conf.hpp>

#include <cstdlib>
#include <utility>
#include <sstream>

#include <log/log_logger.hpp>

namespace sps {

error_t FileReader::read_line(const std::string &name, const std::string arg, LineType &type) {
    return SUCCESS;
}

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

            // sp_debug("set string { %s->%s } success", name, val);
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

ConfigInstant::ConfigInstant(const char* name, PConfigObject obj, const ConfigOption* opts, bool is_root) {
    this->name    = name;
    this->is_root = is_root;
    this->opts    = opts;
    this->obj     = std::move(obj);
}

error_t ConfigInstant::set_default() {
    error_t ret = SUCCESS;

    for (int i = 0;  (opts[i].name); ++i) {
        auto  opt        =  opts+i;
        const char* val  =  opt->default_val.str;
        if ((ret = opt->opt_set(obj.get(), val, strlen(val))) != SUCCESS) {
            sp_error("failed set opt ret:%d", ret);
            return ret;
        }
        sp_debug("set default %s { %s->%s } success", name, opt->name, val);
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
        sp_debug("%d. %s", line, strline.c_str());

        ++line;
        int i = 0;

        if (line_type == SUBMODULE_END || line_type == ROOT_END) {
            break;
        }

        while (opts[i].name && name != opts[i].name) ++i;

        if (!opts[i].name) {
            // sp_error("skip line:%d, %s, %s, %s", line, strline.c_str(), name.c_str(), arg.c_str());
            continue;
        }
        switch (line_type) {
            case ITEM:
                if (opts[i].type == CONF_OPT_TYPE_SUBMODULE) {
                    sp_error("item expect line:%d, %s %s", line, name.c_str(), arg.c_str());
                    return ERROR_CONFIG_PARSE_INVALID;
                }

                opts[i].opt_set(obj.get(), arg.c_str(), arg.size());
                // sp_debug("set %s { %s->%s } success", this->name, opts[i].name, arg.c_str());

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

        arg = "";
        name = "";
    }


    if (ret != SUCCESS && ret != ERROR_IO_EOF) {
        sp_error("Failed read line:%d, ret:%d", line, ret);
        return ret;
    }

    if ((is_root && line_type == ROOT_END) || (!is_root && line_type == SUBMODULE_END)) {
        return SUCCESS;
    }

    sp_error("unexpected end line:%d, %s %s", line, name.c_str(), arg.c_str());
    return ERROR_CONFIG_PARSE_INVALID;
}

error_t ConfigInstant::parse(const std::string &line, std::string &cmd, std::string &arg, LineType &lt) {
    std::stringstream ss;
    ss.str(line);

    std::vector<std::string> args;

    bool found_end = false;

    while (!ss.eof()) {
        std::string tmp_arg;
        ss >> tmp_arg;

        if (!tmp_arg.empty()) {
            args.push_back(tmp_arg);
        } else if (found_end) {
            sp_error("Fatal second end flag %s", line.c_str());
            return ERROR_CONFIG_PARSE_INVALID;
        } else {
            if (tmp_arg[tmp_arg.size()-1] == ';' ||
                    tmp_arg[tmp_arg.size()-1] == '{' ||
                    tmp_arg[tmp_arg.size()-1] == '}') {
                found_end = true;
            }
        }
    }

    if (args.empty()) {
        return SUCCESS;
    }

    if (args.size() == 1) {
        if (args[0] == "}") {
            lt = SUBMODULE_END;
            return SUCCESS;
        } else {
            sp_error("unknown line type %s.", line.c_str());
            return ERROR_CONFIG_PARSE_INVALID;
        }
    }

    cmd = args[0];
    if (cmd == "{" || cmd == "}") {
        sp_error("unknown cmd line type %s.", line.c_str());
        return ERROR_CONFIG_PARSE_INVALID;
    }

    auto last_arg = args[args.size()-1];

    if (last_arg[last_arg.size()-1] == ';') {
        lt = ITEM;
    } else if (last_arg[last_arg.size()-1] == '{') {
        lt = SUBMODULE_START;
    } else if (last_arg[last_arg.size()-1] == '}') {
        lt = SUBMODULE_END;
    } else {
        sp_error("unknown line type :%s", line.c_str());
        return ERROR_CONFIG_PARSE_INVALID;
    }

    for (int i = 1; i < args.size() - 1; ++i) {
        if (arg.empty()) {
            arg += args[i];
        } else {
            arg += " " + args[i];
        }
    }

    if (lt != SUBMODULE_START && lt != SUBMODULE_END) {
        if (*args.rbegin() != ";") {
            arg += " " + args.rbegin()->substr(0, args.rbegin()->size()-1);
        }
    }

    sp_debug("parse line %s->%s.", cmd.c_str(), arg.c_str());

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
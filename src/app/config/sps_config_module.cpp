#include <app/config/sps_config_module.hpp>
#include <log/sps_log.hpp>

#include <sstream>

namespace sps {

IModule::IModule(std::string module_type, std::string module_name, const ConfigOption *opts, PIModule parent) {
    this->module_type   = std::move(module_type);
    this->module_name    = std::move(module_name);
    this->opts   = opts;
    this->parent = std::move(parent);
}

error_t IModule::pre_conf() {
    return SUCCESS;
}

error_t IModule::post_conf() {
    return SUCCESS;
}

error_t IModule::post_sub_module(PIModule sub) {
    return SUCCESS;
}

error_t IModule::init_conf(PIReader rd) {
    auto        cmd_type  = EMPTY;
    error_t     ret       = SUCCESS;
    std::string ctx_line;

    conf = create_conf();     // create conf
    if (!conf) {
        sp_error("create conf ctx failed");
        exit(-1);
        return ret;
    }

    ret = pre_conf();                // prepare for conf
    if (ret != SUCCESS) {
        sp_error("pre conf ctx failed");
        return ret;
    }

    if ((ret = set_default()) != SUCCESS) {  // set default values
        sp_error("set default ctx failed");
        return ret;
    }

    do {    // set config values
        std::string name;
        std::string arg;

        if ((ret = rd->read_line(ctx_line)) != SUCCESS) {
            break;
        }

        if ((ret = parse(ctx_line, name, arg, cmd_type)) != SUCCESS) {
            sp_error("Failed parse line:%d, %s.", rd->cur_line_num(), ctx_line.c_str());
            return ret;
        }

        if (cmd_type == EMPTY || cmd_type == COMMENT) {
            continue;
        } else if (cmd_type == SUBMODULE_END) { // 模块已经处理完成直接，退出
            break;
        }

        int i = 0;
        while (opts[i].name && name != opts[i].name) ++i;

        if (!opts[i].name) {
            sp_warn("skip line:%d, %s, %s, %s", rd->cur_line_num(), ctx_line.c_str(), name.c_str(), arg.c_str());
            continue;
        }

        switch (cmd_type) {
            case ITEM:
                if (opts[i].type == CONF_OPT_TYPE_SUBMODULE) {
                    sp_error("item expect line:%d, %s %s", rd->cur_line_num(), name.c_str(), arg.c_str());
                    return ERROR_CONFIG_PARSE_INVALID;
                }

                opts[i].opt_set(conf.get(), arg.c_str(), arg.size());
                // sp_debug("set %s { %s->%s } success", this->name, opts[i].name, arg.c_str());

                break;
            case SUBMODULE_START: {
                if (opts[i].type != CONF_OPT_TYPE_SUBMODULE) {
                    sp_error("submodule expect line:%d, %s %s", rd->cur_line_num(), name.c_str(),
                             arg.c_str());
                    return ERROR_CONFIG_PARSE_INVALID;
                }

                auto sub = SingleInstance<ModuleFactoryRegister>::get_instance()
                        .create(name, arg, shared_from_this());

                ret = sub->init_conf(rd);

                if (ret != SUCCESS) {
                    sp_error("parse sub module %s %s failed", name.c_str(), arg.c_str());
                    return ret;
                }

                ret = post_sub_module(sub);

                if (ret != SUCCESS) {
                    sp_error("post sub module %s %s failed", name.c_str(), arg.c_str());
                    return ret;
                }

                sp_info("success add submodule %s->%s", module_type.c_str(), sub->module_type.c_str());
                break;
            }
            default:
                sp_error("submodule expect line:%d, %s %s", rd->cur_line_num(), name.c_str(), arg.c_str());
                exit(-1); // illegal be here
        }

    } while(ret == SUCCESS);

    if (ret != SUCCESS && ret != ERROR_IO_EOF) {
        sp_error("Failed read line:%d, ret:%d", rd->cur_line_num(), ret);
        return ret;
    }

    if (("core" == module_type && ret != ERROR_IO_EOF) || ("core" != module_type && cmd_type != SUBMODULE_END)) {
        sp_error("module_type:%s, cmd_type:%d, unexpected end line:%d, %s, ret:%d",
                module_type.c_str(), cmd_type, rd->cur_line_num(), ctx_line.c_str(), ret);
        return ERROR_CONFIG_PARSE_INVALID;
    }

    ret = post_conf();  // post the value

    if (ret != SUCCESS) {
        return ret;
    }

    return SUCCESS;
}

error_t IModule::parse(const std::string &line, std::string &cmd, std::string &arg,
                                LineType &lt) {
    std::stringstream ss;
    ss.str(line);

    std::vector<std::string> args;
    bool found_end = false;

    while (!ss.eof()) { // split the args
        std::string tmp_arg;
        ss >> tmp_arg;

        if (tmp_arg.empty()) {
            continue;
        }

        if (found_end) {
            sp_error("Fatal at least two end flag found %s.", line.c_str());
            return ERROR_CONFIG_PARSE_INVALID;
        }

        char last = tmp_arg[tmp_arg.size() - 1];

        switch (last) {
            case ';':
                found_end = true;
                lt = ITEM;
                break;
            case '{':
                found_end = true;
                lt = SUBMODULE_START;
                break;
            case '}':
                found_end = true;
                lt = SUBMODULE_END;
                break;
            default:
                break;
        }
        args.push_back(tmp_arg);
    }

    if (args.empty()) { // empty line
        lt = EMPTY;
        return SUCCESS;
    }

    if (args[0][0] == '#') {
        lt = COMMENT;
        return SUCCESS;
    }

    if (!found_end) { // no end flag
        sp_error("Fatal no end flag %s", line.c_str());
        return ERROR_CONFIG_PARSE_INVALID;
    }

    if (args.size() == 1 && lt == SUBMODULE_END) {
        return SUCCESS;
    }

    if (args.size() == 1) {
        sp_error("unknown only one arg line type  %s.", line.c_str());
        return ERROR_CONFIG_PARSE_INVALID;
    }

    cmd = args[0];
    if (cmd == "{" || cmd == "}") {
        sp_error("unknown cmd line type %s.", line.c_str());
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
            arg += (arg.empty() ? std::string("") : std::string(" ")) +
                   args.rbegin()->substr(0, args.rbegin()->size() - 1);
        }
    }

    sp_debug("parse line (%s)->(%s) size:%lu", cmd.c_str(), arg.c_str(), args.size());
    return SUCCESS;
}

error_t IModule::set_default() {
    error_t ret = SUCCESS;

    for (int i = 0; (opts[i].name); ++i) {
        auto opt = opts + i;

        if (opt->type == CONF_OPT_TYPE_SUBMODULE) {
            continue;
        }

        const char *val = opt->default_val.str;
        if ((ret = opt->opt_set(conf.get(), val, strlen(val))) != SUCCESS) {
            sp_error("failed set opt ret:%d", ret);
            return ret;
        }
    }

    return ret;
}

PIModule ModuleFactoryRegister::create(const std::string &name, const std::string& arg, PIModule parent) {
    auto f = this->get(name);
    if (!f) {
        sp_error("unknown sub module name:%s", name.c_str());
        exit(1);
    }
    return (*f)->create(name, arg, std::move(parent));
}

}
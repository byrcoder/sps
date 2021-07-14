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

/**
 * module X consists of followings:
 * 1. Conf${X}Ctx reload context
 * 2. ${X}Module
 *     a. create_conf required, impl Conf${X} context
 *     b. pre_conf optional, do something after create_conf and before init_conf default nothing
 *     c. post_sub_module optional, do something after a submodule post_conf.
 *     d. post_conf optional, do something after the init_conf success
 *     e. install optional, do something startup such as starting a server
 * 3. ${X}ModuleFactory how ${X}Module create work as reflection
 *
 *  module layer show such as
 *                       ++++++++++++++++
 *                       + root options +
 *                       ++++++++++++++++ \
 *                     /        |          \
 *                    /         |           \
 *            ++++++++++++  ++++++++++++++   \ +++++++++++++++++++
 *            + upstream +  +  http/rtmp +     +   host (global) +
 *            ++++++++++++  ++++++++++++++     ++++++++++++++++++
 *                          |            |      /
 *                          |            |     /
 *                       ++++++++++  ++++++++ /
 *                       + server +  + host +
 *                       ++++++++++  ++++++++
 *                                      |
 *                                      |
 *                                   ++++++++++
 *                                   + stream +
 *                                   ++++++++++
 *
 */

#ifndef SPS_CONFIG_MODULE_HPP
#define SPS_CONFIG_MODULE_HPP

#include <cstdio>
#include <cctype>

#include <algorithm>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <utility>

#include <sps_module_opt.hpp>

#include <sps_log.hpp>

namespace sps {

struct ConfCtx {
};
typedef std::shared_ptr<ConfCtx> PConfCtx;

class IModule;
typedef std::shared_ptr<IModule> PIModule;

typedef std::string ModuleType;

class IModule : public std::enable_shared_from_this<IModule> {
 public:
    IModule(ModuleType module_type, std::string module_name,
            const ConfigOption* opts, PIModule parent);
    virtual ~IModule() = default;

 public:
    virtual PConfCtx create_conf() = 0;

 public:
    // every thing work in
    virtual error_t install();

 public:
    virtual error_t pre_conf();

    virtual error_t post_conf();

    virtual error_t post_sub_module(PIModule sub);

    virtual error_t merge(PIModule& module);

    error_t init_conf(PIReader rd);

    error_t parse(const std::string &line, std::string &cmd, std::string &arg,
                  LineType &lt);

 public:
    bool is_module(const std::string& name);

 private:
    error_t set_default();

 public:
    ModuleType             module_type;
    std::string            module_name;
    PConfCtx               conf;
    const ConfigOption*    opts;
    PIModule               parent;

 public:
    std::map<ModuleType, std::list<PIModule> > subs;
};

class IModuleFactory {
 public:
    virtual ~IModuleFactory() = default;
    virtual PIModule create(const std::string& module, const std::string& arg,
                            PIModule parent) = 0;

 public:
    std::string module;
};

typedef std::shared_ptr<IModuleFactory> PIModuleFactory;

class ModuleFactoryRegister :
        public KeyRegisters<std::string, PIModuleFactory> {
 public:
    PIModule create(const std::string& name, const std::string& arg,
                    PIModule parent);

 private:
    std::map<std::string, PIModuleFactory> factories;
};

#define MODULE_CONSTRUCT(N, opt) \
    N##Module(std::string module_type, std::string module_name, \
              PIModule parent): \
        IModule(std::move(module_type), std::move(module_name), \
                opt, std::move(parent)) { \
    }

#define MODULE_CREATE_CTX(N) \
    PConfCtx create_conf() override { \
        return std::make_shared<N##ConfCtx>(); \
    }

#define MODULE_FACTORY(N) \
    class  N##ModuleFactory : public IModuleFactory { \
     public: \
        N##ModuleFactory() {  \
            module = #N; \
            transform(module.begin(), module.end(), module.begin(), tolower); \
        } \
     public: \
        PIModule create(const std::string& module, \
                        const std::string& arg, PIModule parent) { \
            sp_debug("===================== %s create %s module:%s, " \
                     "arg:%s=================================", \
                      parent ? parent->module_type.c_str() : "root", \
                      #N, module.c_str(), arg.c_str()); \
            return std::make_shared<N##Module> (module, arg, parent); \
        } \
    };
}  // namespace sps

#endif  // SPS_CONFIG_MODULE_HPP

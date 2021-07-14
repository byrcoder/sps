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

#ifndef SPS_HOST_GLOBAL_MODULE_HPP
#define SPS_HOST_GLOBAL_MODULE_HPP

#include <memory>
#include <vector>

#include <sps_host_module.hpp>

namespace sps {

struct GlobalHostConfCtx : public ConfCtx {

};

#define OFFSET(x) offsetof(GlobalHostConfCtx, x)
static const ConfigOption global_host_options[] = {
        {"host",            "host sub module",       0,             CONF_OPT_TYPE_SUBMODULE,   { .str = "" }, },
        {nullptr }
};
#undef OFFSET

class GlobalHostModule;
typedef std::shared_ptr<GlobalHostModule> PGlobalHostModule;

class GlobalHostModule : public IModule {
 public:
    static std::vector<PGlobalHostModule> global_hosts;

 public:
    MODULE_CONSTRUCT(GlobalHost, global_host_options);

    MODULE_CREATE_CTX(GlobalHost);

    error_t post_sub_module(PIModule sub) override;

    error_t post_conf() override;

 public:
    std::vector<PHostModule> host_modules;
};

MODULE_FACTORY(GlobalHost)

}  // namespace sps

#endif  // SPS_HOST_GLOBAL_MODULE_HPP

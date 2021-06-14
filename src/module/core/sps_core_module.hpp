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

#ifndef SPS_CORE_MODULE_HPP
#define SPS_CORE_MODULE_HPP

#include <sps_module.hpp>
#include <sps_http_module.hpp>

#include <sps_upstream_module.hpp>

namespace sps {

struct CoreConfCtx : public ConfCtx {
    std::string comment;
};

#define OFFSET(x) offsetof(CoreConfCtx, x)
static const ConfigOption core_options[] = {
        { "comment",  "comment", OFFSET(comment),        CONF_OPT_TYPE_STRING,    {.str = "core"} },
        { "upstream", "upstream submodule", 0,    CONF_OPT_TYPE_SUBMODULE, {.str = "upstream"} },
        { "http",     "http submodule", 0,        CONF_OPT_TYPE_SUBMODULE, {.str = "upstream"} },
        { nullptr }
};
#undef OFFSET

class CoreModule : public IModule {
 public:
    MODULE_CONSTRUCT(Core, core_options)

    MODULE_CREATE_CTX(Core);

    error_t post_sub_module(PIModule sub) override;

 public:
    std::list<PHttpModule>      http_modules;
    std::list<PUpStreamModule>  upstream_modules;
};

typedef std::shared_ptr<CoreModule> PCoreModule;

MODULE_FACTORY(Core)

}

#endif  // SPS_CORE_MODULE_HPP

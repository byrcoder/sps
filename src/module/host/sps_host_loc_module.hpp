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

#ifndef SPS_HOST_LOC_MODULE_HPP
#define SPS_HOST_LOC_MODULE_HPP

#include <string>
#include <sps_module.hpp>

namespace sps {

// TODO: ADD add location for request

struct LocationConfCtx : public ConfCtx {
    std::string pattern;    // 匹配模式或者路径
    std::string proxy_pass; // 代理路径
};

#define OFFSET(x) offsetof(LocationConfCtx, x)
static const ConfigOption loc_options[] = {
        {"pattern",        "location name",    OFFSET(pattern),     CONF_OPT_TYPE_STRING, { .str = "/" }, },
        {"proxy_pass",     "proxy pass",       OFFSET(proxy_pass),  CONF_OPT_TYPE_STRING, { .str = "" }, },
        {nullptr }
};
#undef OFFSET

class LocationModule : public IModule {
 public:
    MODULE_CONSTRUCT(Location, loc_options);

    MODULE_CREATE_CTX(Location);
};
typedef std::shared_ptr<LocationModule> PLocationModule;

MODULE_FACTORY(Location)
class LocationModuleFactory;

}

#endif  // SPS_HOST_LOC_MODULE_HPP
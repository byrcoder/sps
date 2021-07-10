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

#ifndef SPS_ROOT_MODULE_HPP
#define SPS_ROOT_MODULE_HPP

#include <memory>
#include <string>

#include <sps_module.hpp>
#include <sps_http_module.hpp>
#include <sps_rtmp_module.hpp>

#include <sps_upstream_module.hpp>

namespace sps {

struct RootConfCtx : public ConfCtx {
    std::string comment;
};

#define OFFSET(x) offsetof(RootConfCtx, x)
static const ConfigOption core_options[] = {
        {nullptr}
};
#undef OFFSET

class RootModule : public IModule {
 public:
    MODULE_CONSTRUCT(Root, core_options)

    MODULE_CREATE_CTX(Root);
};

typedef std::shared_ptr<RootModule> PRootModule;

MODULE_FACTORY(Root)

}  // namespace sps

#endif  // SPS_ROOT_MODULE_HPP

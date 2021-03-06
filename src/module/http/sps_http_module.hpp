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

#ifndef SPS_HTTP_MODULE_HPP
#define SPS_HTTP_MODULE_HPP

#include <set>
#include <memory>
#include <string>

#include <sps_module.hpp>
#include <sps_server_module.hpp>

namespace sps {

struct HttpConfCtx : public ConfCtx {
    int            keepalive_timeout;
    std::string    access_log;
};

#define OFFSET(x) offsetof(HttpConfCtx, x)
static const ConfigOption http_options[] = {
        {"keepalive_timeout",   "location name",    OFFSET(keepalive_timeout),     CONF_OPT_TYPE_INT,    { .str = "10" }, },
        {"access_log",          "proxy pass",       OFFSET(access_log),            CONF_OPT_TYPE_STRING, { .str = "access.log" },   },
        {"server",              "server module",   0,              CONF_OPT_TYPE_SUBMODULE, { .str = "" },   },
        {nullptr }
};
#undef OFFSET

class HttpModule;
typedef std::shared_ptr<HttpModule> PHttpModule;

class HttpModule : public IModule {
 public:
    // merge http servers with same listen port
    static std::list<PServerModule> http_servers;

 public:
    MODULE_CONSTRUCT(Http, http_options);

    MODULE_CREATE_CTX(Http);

    error_t post_sub_module(PIModule sub) override;

    error_t merge(PIModule& module) override;

 public:
    error_t install() override;
};


MODULE_FACTORY(Http)
class HttpModuleFactory;

}  // namespace sps

#endif  // SPS_HTTP_MODULE_HPP

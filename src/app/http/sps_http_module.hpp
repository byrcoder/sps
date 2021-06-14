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

#ifndef SPS_HTTP_CONFIG_HPP
#define SPS_HTTP_CONFIG_HPP

#include <string>

#include <app/config/sps_config_module.hpp>
#include <app/server/sps_server_module.hpp>

namespace sps {

using namespace std;

struct HttpConfCtx : public ConfCtx {
    int       keepalive_timeout;
    string    access_log;
};

#define OFFSET(x) offsetof(HttpConfCtx, x)
static const ConfigOption http_options[] = {
        {"keepalive_timeout",   "location name",    OFFSET(keepalive_timeout),     CONF_OPT_TYPE_INT,    { .str = "10" }, },
        {"access_log",          "proxy pass",       OFFSET(access_log),            CONF_OPT_TYPE_STRING, { .str = "access.log" },   },
        {"server",              "server sub module",       0,              CONF_OPT_TYPE_SUBMODULE, { .str = "" },   },

        {nullptr }
};
#undef OFFSET

class HttpModule : public IModule {
 public:
    MODULE_CONSTRUCT(Http, http_options);

    MODULE_CREATE_CTX(Http);

    error_t post_sub_module(PIModule sub) override;

 public:
    std::list<PServerModule> server_modules;
};

typedef std::shared_ptr<HttpModule> PHttpModule;

MODULE_FACTORY(Http)
class HttpModuleFactory;

}

#endif  // SPS_HTTP_CONFIG_HPP

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

#ifndef SPS_SRT_MODULE_HPP
#define SPS_SRT_MODULE_HPP

#include <list>
#include <memory>
#include <string>
#include <utility>

#include <sps_module.hpp>
#include <sps_server_module.hpp>

#ifdef SRT_ENABLED

namespace sps {

using namespace std;

struct SrtConfCtx : public ConfCtx {
    int            keepalive_timeout;
    string         access_log;
    int            srt_tlpktdrop;
    int            srt_nakreport;
    string         srt_transtype;
};

#define OFFSET(x) offsetof(SrtConfCtx, x)
static const ConfigOption srt_options[] = {
        {"keepalive_timeout",   "location name",    OFFSET(keepalive_timeout),     CONF_OPT_TYPE_INT,    { .str = "10" }, },
        {"access_log",          "proxy pass",       OFFSET(access_log),            CONF_OPT_TYPE_STRING, { .str = "access.log" },   },

        {"srt_tlpktdrop",       "srt tlpktdrop",       OFFSET(srt_tlpktdrop),      CONF_OPT_TYPE_INT, { .str = "0" },   },
        {"srt_nakreport",       "srt nakreport",       OFFSET(srt_nakreport),      CONF_OPT_TYPE_INT, { .str = "0" },   },
        {"srt_transtype",       "srt transtype",       OFFSET(srt_transtype),      CONF_OPT_TYPE_STRING, { .str = "live" },   },

        {"server",              "server sub module",       0,              CONF_OPT_TYPE_SUBMODULE, { .str = "" },   },

        {nullptr }
};
#undef OFFSET

class SrtModule : public IModule {
 public:
    MODULE_CONSTRUCT(Srt, srt_options);

    MODULE_CREATE_CTX(Srt);

    error_t post_sub_module(PIModule sub) override;

    error_t merge(PIModule& module) override;

 public:
    error_t install() override;

 public:
    std::list<PServerModule> srt_servers;
};

typedef std::shared_ptr<SrtModule> PSrtModule;

MODULE_FACTORY(Srt)
class SrtModuleFactory;

}  // namespace sps

#endif

#endif  // SPS_SRT_MODULE_HPP

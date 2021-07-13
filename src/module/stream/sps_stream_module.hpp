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

#ifndef SPS_STREAM_CONFIG_HPP
#define SPS_STREAM_CONFIG_HPP

#include <sps_module.hpp>

#include <memory>
#include <string>

namespace sps {

struct StreamConfCtx : public ConfCtx {
    std::string avformat;
    bool        edge;
    std::string pass_proxy;
};

#define OFFSET(x) offsetof(StreamConfCtx, x)
static const ConfigOption stream_options[] = {
        { "avformat",          "rtmp/flv/proxy",        OFFSET(avformat),            CONF_OPT_TYPE_STRING, {.str = "-"} },
        { "edge",              "edge/publish",          OFFSET(edge),                CONF_OPT_TYPE_BOOL,   {.str = "on"} },
        { "pass_proxy",        "edge/publish",          OFFSET(pass_proxy),          CONF_OPT_TYPE_STRING, {.str = "-"} },
        { nullptr }
};
#undef OFFSET

class StreamModule : public IModule {
 public:
    MODULE_CONSTRUCT(Stream, stream_options)

    MODULE_CREATE_CTX(Stream);
};

typedef std::shared_ptr<StreamModule> PStreamModule;

MODULE_FACTORY(Stream)

}  // namespace sps

#endif  // SPS_STREAM_CONFIG_HPP

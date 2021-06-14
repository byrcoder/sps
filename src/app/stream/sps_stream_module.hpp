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

#include <app/module/sps_module.hpp>

namespace sps {

struct StreamConfCtx : public ConfCtx {
    std::string play_avformat;
    std::string upstream_avformat;
    bool        edge;
};

#define OFFSET(x) offsetof(StreamConfCtx, x)
static const ConfigOption stream_options[] = {
        { "play_avformat",     "play avformat",         OFFSET(play_avformat),        CONF_OPT_TYPE_STRING, {.str = "-"} },
        { "upstream_avformat", "upstream avformat",     OFFSET(upstream_avformat),    CONF_OPT_TYPE_STRING, {.str = "-"} },
        { "edge",              "upstream avformat",     OFFSET(edge),                 CONF_OPT_TYPE_BOOL,   {.str = "off"} },
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

}

#endif  // SPS_STREAM_CONFIG_HPP

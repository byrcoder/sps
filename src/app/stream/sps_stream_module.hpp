#ifndef SPS_STREAM_CONFIG_HPP
#define SPS_STREAM_CONFIG_HPP

#include <app/config/sps_config_module.hpp>

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

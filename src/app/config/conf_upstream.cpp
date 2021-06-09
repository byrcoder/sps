#include <app/config/conf_upstream.hpp>

namespace sps {

#define OFFSET(x) offsetof(ConfUpstream, x)
const ConfigOption upstream_options[] = {
        {"name",       "upstream name",    OFFSET(name),    CONF_OPT_TYPE_STRING, { .str = "" }, },
        {"server",     "server array",     OFFSET(server),  CONF_OPT_TYPE_STRING, { .str = "" }, },
        {"keepalive",  "timeout to keep",  OFFSET(server),  CONF_OPT_TYPE_INT,    { .i64 = 10  }, },
        { nullptr }
};

PConfInstant UpstreamConfigInstantFactory::create() {
    return std::make_shared<ConfigInstant>(std::unique_ptr<ConfUpstream>(new ConfUpstream()), upstream_options);
}

}
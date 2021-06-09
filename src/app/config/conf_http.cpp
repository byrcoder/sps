#include <app/config/conf_http.hpp>

namespace sps {

#define OFFSET(x) offsetof(ConfHttp, x)
static const ConfigOption http_options[] = {
        {"keepalive_timeout",        "location name",    OFFSET(keepalive_timeout),     CONF_OPT_TYPE_INT, { .i64 = 10 }, },
        {"access_log",               "proxy pass",       OFFSET(access_log),            CONF_OPT_TYPE_STRING, { .str = "" }, },
        {nullptr }
};

}

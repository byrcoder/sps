#ifndef SPS_CONF_HTTP_HPP
#define SPS_CONF_HTTP_HPP

#include <app/config/conf.hpp>
#include <string>

namespace sps {

using namespace std;

class ConfUpstream : public IConfigInstant {
 public:
    string  name;
    string  server;
    int     keepalive;
};

#define OFFSET(x) offsetof(ConfUpstream, x)
const ConfigOption[] upstream_options = {
        {"name",       "upstream name",    OFFSET(name),    CONF_OPT_TYPE_STRING, { .str = "" }, },
        {"server",     "server array",     OFFSET(server),  CONF_OPT_TYPE_STRING, { .str = "" }, },
        {"keepalive",  "timeout to keep",  OFFSET(server),  CONF_OPT_TYPE_INT,    { .i64 = 10  }, },
        {nullptr }
};
#undef OFFSET(x)

#ifndef OFFSET(x)
class ConfLocation {
 public:

};

class ConfServer {
 public:
    int       listen_port;
    string    server_name;   // host
};

class ConfHttp {
 public:
    int       keepalive_timeout;
    string    access_log;
};

}

#endif //SPS_CONF_HTTP_HPP

#include <app/config/conf_server.hpp>

namespace sps {

#define OFFSET(x) offsetof(ConfServer, x)
static const ConfigOption server_options[] = {
        {"listen_port",     "location name",    OFFSET(listen_port),  CONF_OPT_TYPE_INT,    { .i64 = 80 }, },
        {"server_name",     "proxy pass",       OFFSET(server_name),  CONF_OPT_TYPE_STRING, { .str = "" }, },
        {nullptr }
};

ConfServer::ConfServer() {

}


}

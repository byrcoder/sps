#include <app/config/conf_location.hpp>

namespace sps {

#define OFFSET(x) offsetof(ConfLocation, x)
static const ConfigOption location_options[] = {
        {"pattern",        "location name",    OFFSET(pattern),     CONF_OPT_TYPE_STRING, { .str = "/" }, },
        {"proxy_pass",     "proxy pass",       OFFSET(proxy_pass),  CONF_OPT_TYPE_STRING, { .str = "" }, },
        {nullptr }
};

ConfLocation::ConfLocation()  {

}


}

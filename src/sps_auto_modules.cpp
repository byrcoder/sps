#ifndef SPS_AUTO_MODULES_HPP
#define SPS_AUTO_MODULES_HPP

#include <app/core/sps_core_config.hpp>
#include <app/config/sps_config_host.hpp>
#include <app/config/sps_config_loc.hpp>

#include <app/http/sps_http_config.hpp>

#include <app/server/sps_server_config.hpp>

#include <app/upstream/sps_upstream_config.hpp>

#define MODULE_INSTANCE(NAME) (std::make_shared<sps::NAME##ModuleFactory>())

sps::PIModuleFactory modules[] = {
    MODULE_INSTANCE(Core),
    MODULE_INSTANCE(Host),
    MODULE_INSTANCE(Location),
    MODULE_INSTANCE(Http),
    MODULE_INSTANCE(Server),
    MODULE_INSTANCE(UpStream),
    nullptr
};


#endif  // SPS_AUTO_MODULES_HPP

#ifndef SPS_AUTO_MODULES_HPP
#define SPS_AUTO_MODULES_HPP

#include <app/config/conf_core.hpp>
#include <app/config/conf_host.hpp>
#include <app/config/conf_loc.hpp>

#include <app/http/conf_http.hpp>

#include <app/server/conf_server.hpp>

#include <app/upstream/conf_upstream.hpp>

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

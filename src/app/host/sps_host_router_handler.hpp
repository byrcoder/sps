#ifndef SPS_HOST_ROUTER_HANDLER_HPP
#define SPS_HOST_ROUTER_HANDLER_HPP

#include <app/host/sps_host_phase_handler.hpp>
#include <app/server/sps_server_module.hpp>

namespace sps {

class HostRouterPhaseHandler : public IPhaseHandler {
 public:
    explicit HostRouterPhaseHandler(PHostModulesRouter router,
                                    PIPhaseHandler default_handler,
                                    PIPhaseHandler not_found_handler);

 public:
    error_t handler(HostPhaseCtx& ctx) override;

 private:
    PHostModulesRouter  router;
    PIPhaseHandler      host_handler;
    PIPhaseHandler      not_found_handler;
};

}

#endif  // SPS_HOST_ROUTER_HANDLER_HPP

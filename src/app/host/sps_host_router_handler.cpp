#include <app/host/sps_host_router_handler.hpp>

namespace sps {

HostRouterPhaseHandler::HostRouterPhaseHandler(PHostModulesRouter router,
        PIPhaseHandler df, PIPhaseHandler not_found_handler) :
                                                IPhaseHandler("host-router-handler") {
    this->router          = std::move(router);
    this->host_handler    = std::move(df);
    this->not_found_handler = std::move(not_found_handler);
}

error_t HostRouterPhaseHandler::handler(HostPhaseCtx &ctx) {
    ctx.host = router->find_host(ctx.req->host);

    if (!ctx.host) {
        sp_error("Not found host:%s", ctx.req->host.c_str());
        if (not_found_handler) {
            return not_found_handler->handler(ctx);
        }
        return ERROR_HOST_NOT_EXISTS;
    }

    return host_handler->handler(ctx);
}

}

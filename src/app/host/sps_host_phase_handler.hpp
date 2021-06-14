#ifndef SPS_HOST_PHASE_HANDLER_HPP
#define SPS_HOST_PHASE_HANDLER_HPP

#include <app/url/sps_url.hpp>
#include <app/host/sps_host_module.hpp>
#include <net/sps_net_socket.hpp>

namespace sps {

struct HostPhaseCtx {
    HostPhaseCtx(PRequestUrl r, PSocket s);

    PRequestUrl req;
    PSocket     socket;
    std::string ip;
    int         port;
    PHostModule host;
};

/**
 * work as nginx
 */
class IPhaseHandler {
 public:
    explicit IPhaseHandler(const char* name) : name(name) { }
    virtual const char* get_name() { return name; }

 public:
    virtual error_t handler(HostPhaseCtx& ctx) = 0;

 private:
    const char* name;
};

typedef std::shared_ptr<IPhaseHandler> PIPhaseHandler;

}


#endif  // SPS_HOST_PHASE_HANDLER_HPP

#ifndef SPS_HTTP_PHASE_HANDLER_HPP
#define SPS_HTTP_PHASE_HANDLER_HPP

#include <app/url/sps_url.hpp>
#include <app/http/sps_http_parser.hpp>
#include <app/host/sps_host_phase_handler.hpp>
#include <app/server/sps_server_module.hpp>

#include <net/sps_net_socket.hpp>


#define SPS_HTTP_PHASE_CONTINUE 0
#define SPS_HTTP_PHASE_SUCCESS_NO_CONTINUE 1

namespace sps {

// http头部解析
class HttpParsePhaseHandler : public IPhaseHandler {
 public:
    HttpParsePhaseHandler();

 public:
    error_t handler(HostPhaseCtx& ctx) override;
};

// 域名发现
class HttpProxyPhaseHandler : public IPhaseHandler {
 public:
    explicit HttpProxyPhaseHandler();

 public:
    error_t handler(HostPhaseCtx& ctx) override;
};

// 404 响应
class Http404PhaseHandler : public IPhaseHandler, public Single<Http404PhaseHandler> {
 public:
    Http404PhaseHandler();

 public:
    error_t handler(HostPhaseCtx& ctx) override;
};

/**
 * work as nginx
 */
class HttpPhaseHandler : public FifoRegisters<PIPhaseHandler> {
 public:
    error_t handler(HostPhaseCtx& ctx);

 public:
    HttpPhaseHandler();
};

typedef std::shared_ptr<HttpPhaseHandler> PHttpPhaseHandler;

}

#endif  // SPS_HTTP_PHASE_HANDLER_HPP

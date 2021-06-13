#ifndef SPS_HTTP_PHASE_HANDLER_HPP
#define SPS_HTTP_PHASE_HANDLER_HPP

#include <app/url/sps_url.hpp>
#include <app/http/sps_http_parser.hpp>
#include <app/server/sps_server_config.hpp>

#include <net/sps_net_socket.hpp>


#define SPS_HTTP_PHASE_CONTINUE 0
#define SPS_HTTP_PHASE_SUCCESS_NO_CONTINUE 1

namespace sps {

struct HttpPhCtx {
    HttpPhCtx(PRequestUrl r, PSocket s);

    PRequestUrl req;
    PSocket     socket;
    std::string ip;
    int         port;

    std::string session_host;
};

/**
 * work as nginx
 */
class IHttpPhaseHandler {
 public:
    explicit IHttpPhaseHandler(const char* name) : name(name) { }
    virtual const char* get_name() { return name; }

 public:
    virtual error_t handler(HttpPhCtx& ctx) = 0;

 private:
    const char* name;
};

typedef std::shared_ptr<IHttpPhaseHandler> PIHttpPhaseHandler;

class HttpParsePhaseHandler : public IHttpPhaseHandler {
 public:
    HttpParsePhaseHandler();

 public:
    error_t handler(HttpPhCtx& ctx) override;
};

class HttpRouterPhaseHandler : public IHttpPhaseHandler {
 public:
    explicit HttpRouterPhaseHandler(PServerModule ctx);

 public:
    error_t handler(HttpPhCtx& ctx) override;

 private:
    PHostModule find_host_ctx(HttpPhCtx& ctx);

    error_t do_handler(PHostModule& host_ctx, HttpPhCtx& ctx);

 private:
    PServerModule server_ctx;
};

class Http404PhaseHandler : public IHttpPhaseHandler, public Single<Http404PhaseHandler> {
 public:
    Http404PhaseHandler();

 public:
    error_t handler(HttpPhCtx& ctx) override;
};

/**
 * work as nginx
 */
class HttpPhaseHandler : public FifoRegisters<PIHttpPhaseHandler> {
 public:
    error_t handler(HttpPhCtx& ctx);

 public:
    HttpPhaseHandler();
};

typedef std::shared_ptr<HttpPhaseHandler> PHttpPhaseHandler;

}

#endif  // SPS_HTTP_PHASE_HANDLER_HPP

#ifndef SPS_HTTP_PHASE_HANDLER_HPP
#define SPS_HTTP_PHASE_HANDLER_HPP

#include <app/url/url.hpp>
#include <app/http/http_parser.hpp>
#include <net/net_socket.hpp>


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

class HttpProxyPhaseHandler : public IHttpPhaseHandler {
 public:
    HttpProxyPhaseHandler();

 public:
    error_t handler(HttpPhCtx& ctx) override;

 private:
    error_t create_proxy_request(HttpPhCtx& ctx, PRequestUrl& proxy_req);

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
class HttpPhaseHandler : public FifoRegisters<HttpPhaseHandler, PIHttpPhaseHandler> {
 public:
    error_t handler(HttpPhCtx& ctx);

 public:
    HttpPhaseHandler();
};

}

#endif  // SPS_HTTP_PHASE_HANDLER_HPP

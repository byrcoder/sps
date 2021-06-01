#ifndef SPS_PHASE_HANDLER_HPP
#define SPS_PHASE_HANDLER_HPP

#include <app/url.hpp>
#include <app/http/parser.hpp>
#include <net/socket.hpp>

namespace sps {

struct HttpPhCtx {
    HttpPhCtx(PRequestUrl r, PSocket s);

    PRequestUrl req;
    PSocket     socket;
};

/**
 * work as nginx
 */
class IHttpPhaseHandler {
 public:
    virtual ~IHttpPhaseHandler() = default;

 public:
    virtual error_t handler(HttpPhCtx& ctx) = 0;
};

class HttpParsePhaseHandler : public IHttpPhaseHandler {

};

class Http404PhaseHandler : public IHttpPhaseHandler {

 public:
    static Http404PhaseHandler& get_instance();

 public:
    virtual error_t handler(HttpPhCtx& ctx);
};

typedef std::shared_ptr<IHttpPhaseHandler> PIHttpPhaseHandler;

/**
 * work as nginx
 */
class HttpPhaseHandler {
 public:
    static HttpPhaseHandler& get_instance();

 public:
    error_t handler(HttpPhCtx& ctx);

 public:
    std::list<PIHttpPhaseHandler> filters;
};

}

#endif  // SPS_PHASE_HANDLER_HPP

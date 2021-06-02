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

    virtual const char* get_name() = 0;

 public:
    virtual error_t handler(HttpPhCtx& ctx) = 0;
};

typedef std::shared_ptr<IHttpPhaseHandler> PIHttpPhaseHandler;

class HttpParsePhaseHandler : public IHttpPhaseHandler {
 public:
    error_t handler(HttpPhCtx& ctx) override;
    const char* get_name() override;
};

class Http404PhaseHandler : public IHttpPhaseHandler {

 public:
    static Http404PhaseHandler& get_instance();

 public:
    error_t handler(HttpPhCtx& ctx) override;

    const char* get_name() override;
};

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

 private:
    HttpPhaseHandler();
};

}

#endif  // SPS_PHASE_HANDLER_HPP

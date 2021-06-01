#ifndef SPS_FILTER_HPP
#define SPS_FILTER_HPP

#include <app/url.hpp>
#include <app/http/parser.hpp>
#include <net/socket.hpp>

namespace sps {

/**
 * http的业务逻辑处理
 */
class IHttpFilter {
 public:
    virtual ~IHttpFilter() = default;

 public:
    virtual error_t filter(PRequestUrl req, PSocket socket) = 0;
};

class Http404Filter : public IHttpFilter {

 public:
    static Http404Filter& get_instance();

 public:
    virtual error_t filter(PRequestUrl req, PSocket socket);
};

typedef std::shared_ptr<IHttpFilter> PIHttpFilter;

class HttpFilterLink {
 public:
    static HttpFilterLink& get_instance();

 public:
    error_t filter(PRequestUrl req, PSocket socket);

 public:
    std::list<PIHttpFilter> filters;
};

}

#endif  // SPS_FILTER_HPP

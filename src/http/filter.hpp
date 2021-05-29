#ifndef SPS_FILTER_HPP
#define SPS_FILTER_HPP

#include <http/parser.hpp>
#include <net/socket.hpp>

namespace sps {

/**
 * http的业务逻辑处理
 */
class IHttpFilter {
 public:
    virtual ~IHttpFilter() = default;

 public:
    virtual error_t filter(PHttpRequest req, PClientSocket socket) = 0;
};

class Http404Filter : public IHttpFilter {

 public:
    static Http404Filter& get_instance();

 public:
    virtual error_t filter(PHttpRequest req, PClientSocket socket);
};

typedef std::shared_ptr<IHttpFilter> PIHttpFilter;

class HttpFilterLink {
 public:
    static HttpFilterLink& get_instance();

 public:
    error_t filter(PHttpRequest req, PClientSocket socket);

 public:
    std::list<PIHttpFilter> filters;
};

}

#endif  // SPS_FILTER_HPP

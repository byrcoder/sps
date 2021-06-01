#ifndef SPS_UPSTREAM_HPP
#define SPS_UPSTREAM_HPP

#include <memory>

#include <cache/cache.hpp>
#include <co/co.hpp>
#include <net/socket.hpp>
#include <app/url.hpp>
#include <app/stream/protocol.hpp>

namespace sps {

class IUpstream : public ICoHandler {
 public:
    explicit IUpstream(PICacheStream cs);
    ~IUpstream() override;

 public:
    // TODO: fix request
    virtual error_t open_url(PRequestUrl req) = 0;
    virtual error_t open_url(const std::string& url);

 public:
    error_t handler() override;

 public:
    PICacheStream get_cs();

 protected:
    PICacheStream    cs;
    PIProtocol       protocol;
};
typedef std::shared_ptr<IUpstream> PIUpstream;

}

#endif //SPS_UPSTREAM_HPP

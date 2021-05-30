#ifndef SPS_UPSTREAM_HPP
#define SPS_UPSTREAM_HPP

#include <memory>

#include <cache/cache.hpp>
#include <co/co.hpp>
#include <protocol/url.hpp>

namespace sps {

class IUpstream : public ICoHandler {
 public:
    IUpstream(PICacheStream cs);
    virtual ~IUpstream();

 public:
    // TODO: fix request
    virtual error_t init(PRequestUrl req) = 0;
    virtual error_t up() = 0;

 public:
    error_t handler() override;

 public:
    PICacheStream get_cs();

 protected:
    PICacheStream cs;
};
typedef std::shared_ptr<IUpstream> PIUpstream;

}

#endif //SPS_UPSTREAM_HPP

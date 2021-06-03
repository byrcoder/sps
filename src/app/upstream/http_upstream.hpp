#include <cache/cache.hpp>
#include <app/http/client.hpp>
#include <app/upstream/upstream.hpp>

#ifndef SPS_HTTP_UPSTREAM_HPP
#define SPS_HTTP_UPSTREAM_HPP

#include <app/stream/protocol.hpp>

namespace sps {

class HttpUpstream : public IUpstream {
 public:
    explicit HttpUpstream(PICacheStream cs, Protocol p);

 public:
    // TODO: fix request
    error_t open_url(PRequestUrl req, utime_t tm) override; // work as reinit_request
    void abort_request() override;
};

}

#endif //SPS_HTTP_UPSTREAM_HPP

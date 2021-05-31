#include <cache/cache.hpp>
#include <protocol/http/client.hpp>
#include <upstream/upstream.hpp>

#ifndef SPS_HTTP_UPSTREAM_HPP
#define SPS_HTTP_UPSTREAM_HPP

namespace sps {

class HttpUpstream : public IUpstream {
 public:
    explicit HttpUpstream(PICacheStream cs);

 public:
    error_t open_url(PRequestUrl req) override;
};

}

#endif //SPS_HTTP_UPSTREAM_HPP

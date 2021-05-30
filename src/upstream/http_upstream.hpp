#include <cache/cache.hpp>
#include <protocol/http/client.hpp>
#include <upstream/upstream.hpp>

#ifndef SPS_HTTP_UPSTREAM_HPP
#define SPS_HTTP_UPSTREAM_HPP

namespace sps {

class HttpUpstream : public IUpstream {
 public:
    HttpUpstream(PICacheStream cs);

 public:
    error_t init(PRequestUrl req) override;
    error_t up() override;

 private:
    // ClientSocket
};

}

#endif //SPS_HTTP_UPSTREAM_HPP

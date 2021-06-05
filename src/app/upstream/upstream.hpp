#ifndef SPS_UPSTREAM_HPP
#define SPS_UPSTREAM_HPP

#include <memory>

#include <cache/cache.hpp>
#include <co/co.hpp>
#include <net/net_socket.hpp>
#include <app/url/url.hpp>
#include <app/stream/dec.hpp>

/**
 * nginx upstream:
 * 1. create_request
 * 2. reinit_request
 * 3. process_header
 * 4. abort_request
 * 5. finalize_request
 * 6.
 */
namespace sps {

class Upstream;
typedef std::shared_ptr<Upstream> PIUpstream;

class UpstreamContainer : public Registers<UpstreamContainer, PIUpstream> {

};

typedef std::shared_ptr<UpstreamContainer> PIUpstreamContainer;;

class Upstream : public ICoHandler, public std::enable_shared_from_this<Upstream> {
    friend class UpstreamContainer;

 public:
    explicit Upstream(PICacheStream cs, PIAvDemuxer dec, PRequestUrl url);
    ~Upstream() override;

 public:
    // virtual error_t process() = 0; // process work in handler
    virtual void abort_request();

 public:
    error_t handler() override;
    void on_stop() override;


public:
    PICacheStream get_cs();

 protected:
    PICacheStream       cs;
    PRequestUrl         url;
    PIAvDemuxer         decoder;
};

class UpstreamFactory : public Single<UpstreamFactory> {
 public:
    PIUpstream create_upstream(PICacheStream cs, PRequestUrl url); // // work as reinit_request
};

}

#endif //SPS_UPSTREAM_HPP

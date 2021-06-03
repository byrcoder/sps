#ifndef SPS_UPSTREAM_HPP
#define SPS_UPSTREAM_HPP

#include <memory>

#include <cache/cache.hpp>
#include <co/co.hpp>
#include <net/socket.hpp>
#include <app/url.hpp>
#include <app/stream/protocol.hpp>

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

class IUpstream;
typedef std::shared_ptr<IUpstream> PIUpstream;

class IUpstreamContainer {
 public:
    virtual void register_upstream(PIUpstream u) = 0;
    virtual void delete_upstream(PIUpstream u);
};

typedef std::shared_ptr<IUpstreamContainer> PIUpstreamContainer;;

class IUpstream : public ICoHandler, public std::enable_shared_from_this<IUpstream> {
    friend class IUpstreamContainer;

 public:
    explicit IUpstream(PICacheStream cs, Protocol p);
    ~IUpstream() override;

 public:
    // TODO: fix request
    virtual error_t open_url(PRequestUrl req, utime_t tm) = 0; // work as reinit_request
    // virtual error_t process() = 0; // process work in handler
    virtual void abort_request();

 public:
    error_t open_url(const std::string& url, utime_t tm);

 public:
    error_t handler() override;
    void on_stop() override;


public:
    PICacheStream get_cs();

 protected:
    PICacheStream       cs;
    PIProtocol          protocol;
    PIUpstreamContainer container;
    Protocol            p;
};

class IUpStreamFactory {
 public:
    virtual PIUpstream create_upstream(PICacheStream cs, PRequestUrl url) = 0;  // work as create_request
};

class UpstreamFactory : public IUpStreamFactory {
 public:
    PIUpstream create_upstream(PICacheStream cs, PRequestUrl url) override;
};

}

#endif //SPS_UPSTREAM_HPP

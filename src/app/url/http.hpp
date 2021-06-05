#ifndef SPS_APP_URL_HTTP_HPP
#define SPS_APP_URL_HTTP_HPP

#include <app/url/protocol.hpp>

#include <map>
#include <string>


namespace sps {

class HttpProtocol : public IURLProtocol {
 public:
    explicit HttpProtocol() = default;
 public:
    error_t open(PRequestUrl url, Transport p) override;
};

class HttpProtocolFactory : public IUrlProtocolFactory {
 public:
    HttpProtocolFactory();

 public:
    PIURLProtocol create(PRequestUrl url);
};

}

#endif  // SPS_APP_URL_HTTP_HPP

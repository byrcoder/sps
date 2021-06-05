#ifndef SPS_APP_URL_PROTOCOL_HPP
#define SPS_APP_URL_PROTOCOL_HPP

#include <app/url/url.hpp>

#include <net/net_socket.hpp>

namespace sps {

class IURLProtocol : public Socket {
 public:
    IURLProtocol() = default;

 public:
    virtual error_t open(PRequestUrl url, Transport p = DEFAULT) = 0;
    error_t open(const std::string url, Transport p);

 public:
    virtual PResponse response() = 0;
};
typedef std::shared_ptr<IURLProtocol> PIURLProtocol;

class IUrlProtocolFactory  {
 public:
    IUrlProtocolFactory(const char* schema, Transport t);

 public:
    ~IUrlProtocolFactory() = default;

 public:
    virtual bool match(PRequestUrl url);
    virtual PIURLProtocol create(PRequestUrl url) = 0;

 public:
    const char* schema;
    Transport   t;
};

typedef std::shared_ptr<IUrlProtocolFactory> PIUrlProtocolFactory;

class UrlProtocol : public Registers<UrlProtocol, PIUrlProtocolFactory> {
 public:
    PIURLProtocol create(PRequestUrl url);
    PIURLProtocol create(const std::string& url);
};

}

#endif  // SPS_APP_URL_PROTOCOL_HPP

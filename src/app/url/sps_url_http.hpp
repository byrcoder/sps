#ifndef SPS_URL_HTTP_HPP
#define SPS_URL_HTTP_HPP

#include <app/url/sps_url_protocol.hpp>

#include <map>
#include <string>
#include <app/http/sps_http_parser.hpp>

namespace sps {

class HttpProtocol : public IURLProtocol {
 public:
    explicit HttpProtocol() = default;
 public:
    error_t open(PRequestUrl url, Transport p) override;

 public:
    error_t read(void* buf, size_t size, size_t& nread) override;

    error_t read_chunked(void* buf, size_t size, size_t nread);

    error_t read_chunked_length();

    error_t read_chunked_data(void* buf, size_t size, size_t nread);

 public:
    bool eof();

 public:
    PResponse response() override;

 private:
    PHttpResponse rsp;
    size_t  nread = 0;

    size_t nb_chunked_size = 0;
    size_t nb_chunked_left = 0;

    bool is_eof         = false;
};

class HttpProtocolFactory : public IUrlProtocolFactory {
 public:
    HttpProtocolFactory();

 public:
    PIURLProtocol create(PRequestUrl url);
};

}

#endif  // SPS_URL_HTTP_HPP

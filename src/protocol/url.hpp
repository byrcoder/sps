#ifndef SPS_PROTOCOL_URL_HPP
#define SPS_PROTOCOL_URL_HPP

#include <list>
#include <memory>
#include <string>
#include <typedef.hpp>

namespace sps {

struct RequestHeader {
    explicit RequestHeader();
    explicit RequestHeader(const std::string& key, const std::string& value);
    std::string key;
    std::string value;
};

typedef RequestHeader RequestParam;

class RequestUrl;
typedef std::shared_ptr<RequestUrl> PRequestUrl;

class RequestUrl {
 public:
    error_t parse_url(const std::string& url);

 public:
    RequestUrl() = default;

 public:
    virtual const char* get_schema();
    virtual const char* get_url();
    virtual const char* get_host();
    virtual const char* get_path();
    virtual const char* get_params();
    virtual const char* get_method();

    std::string get_header(const std::string& key);
    std::string get_param(const std::string& key);

 public:
    std::string url;
    std::string method;
    std::string schema;
    int         port = 80;
    std::string host;
    std::string path;
    std::string params;
    std::list<RequestParam> pp;
    std::list<RequestHeader> headers;
};

typedef std::shared_ptr<RequestUrl> PRequestUrl;

}

#endif  // SPS_PROTOCOL_URL_HPP

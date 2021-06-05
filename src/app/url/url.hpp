#ifndef SPS_APP_URL_URL_HPP
#define SPS_APP_URL_URL_HPP

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
    static error_t from(const std::string& url, std::shared_ptr<RequestUrl>& req);

 public:
    error_t parse_url(const std::string& url);

 public:
    explicit RequestUrl() = default;

 public:
    virtual const char* get_schema();
    virtual const char* get_url();
    virtual const char* get_host();
    virtual int         get_port();
    virtual const char* get_path();
    virtual const char* get_params();
    virtual const char* get_method();
    virtual const char* get_ext();

    virtual void erase_header(const std::string& key);
    std::string get_header(const std::string& key);
    std::string get_param(const std::string& key);

 public:
    utime_t get_timeout();
    std::string  get_ip();

 public:
    std::string url;
    std::string method;
    std::string schema;
    int         port = 80;
    std::string host;
    std::string path;
    std::string ext;
    std::string params;
    std::list<RequestParam> pp;
    std::list<RequestHeader> headers;

 public:
    utime_t tm = 10 * 1000 * 1000;
    std::string ip;
};

typedef std::shared_ptr<RequestUrl> PRequestUrl;

class Response {
 public:
    virtual ~Response() = default;
};

typedef std::shared_ptr<Response> PResponse;

}

#endif  // SPS_APP_URL_URL_HPP

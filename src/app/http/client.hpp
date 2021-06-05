#ifndef SPS_CLIENT_HPP
#define SPS_CLIENT_HPP

#include <string>
#include <app/http/parser.hpp>
#include <net/io.hpp>

namespace sps {

using namespace std;

// 使用url protocol 替换
__deprecated class HttpClient {
 public:
    virtual ~HttpClient() = default;

 public:
    error_t init(const string& host, int port = 80,
                 utime_t timeout = 3 * 1000 * 1000, const string* ip = nullptr);
    virtual error_t init_io() = 0;

 public:
    error_t get(const string& url, map<string, string>* headers = nullptr);
    error_t post(const std::string& url, const std::string& data, map<string, string> *headers = nullptr);

 public:
    PIReaderWriter get_io();
    int status_code();

 protected:
    PIReaderWriter io;

    string  host;
    int     port;
    string  ip;
    utime_t timeout;
    std::shared_ptr<HttpParserContext> ctx;

 public:
    static std::shared_ptr<HttpClient> create_http_create();
};

__deprecated class StHttpClient : public HttpClient {
 public:
    error_t init_io() override;
};

}

#endif //SPS_CLIENT_HPP

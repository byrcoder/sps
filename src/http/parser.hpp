#ifndef SPS_PARSER_HPP
#define SPS_PARSER_HPP

#include <http_parser.h>

#include <list>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "net/io.hpp"

namespace sps {

enum HttpType {
    REQUEST,
    RESPONSE,
    BOTH     // 这里只有测试使用
};

class HttpHeader {
 public:
    std::string key;
    std::string value;
};

class HttpRequest {
 public:
    std::string url;
    std::string body;
    std::list<HttpHeader> headers;

    std::string method;
    std::string schme;
    int         port = 80;
    std::string host;
    std::string path;
    std::string params;
    std::map<std::string, std::string> pp; // params key=value
};

typedef std::shared_ptr<HttpRequest> PHttpRequest;

class HttpResponse {
 public:
    int   content_length = -1;
    int   status_code    = -1;
    bool  chunked        = false;
    std::list<HttpHeader> headers;
};

typedef std::shared_ptr<HttpResponse> PHttpResponse;

class HttpParserContext {
 public:
    int http_status = -1;
    std::string url;
    std::string body;
    std::list<HttpHeader> headers;
    http_parser http;

    std::string schema;
    std::string host;
    std::string path;
    std::string params;
    std::map<std::string, std::string> pp;
    int         port = 80;

 public:
    // http parse的结果
    int content_length();

    bool is_chunked();

    int status_code();

    const char* method();

 public:
    bool contains(const std::string& key, std::string* value);
    bool contains(const std::string& key, std::vector<std::string>* vs);

 public:
    int parse_url(std::shared_ptr<HttpRequest> &req);
    int dump(std::shared_ptr<HttpResponse>& res);
};

typedef std::shared_ptr<HttpParserContext> PHttpParserContext;

class HttpParser {
 public:
    HttpParser(int max_header = 1024);
    int parse_header(PIReader io, HttpType ht);
    int parse_header(const char* buf, int len, HttpType ht);

 public:
    PHttpParserContext get_ctx();
    PHttpResponse      get_response();
    PHttpRequest       get_request();

 private:
    HttpType http_type;
    int max_header;
    std::unique_ptr<char[]> buf;
    int buf_read;
    HttpHeader head;

    PHttpParserContext ctx;
    PHttpRequest       req;
    PHttpResponse      res;

 public:
    static int on_message_begin(http_parser* hp);
    static int on_url(http_parser* hp, const char *at, size_t length);
    static int on_status(http_parser* hp, const char *at, size_t length);
    static int on_header_field(http_parser* hp, const char *at, size_t length);
    static int on_header_value(http_parser* hp, const char *at, size_t length);
    static int on_headers_complete(http_parser* hp);
    static int on_body(http_parser* hp, const char *at, size_t length);
    static int on_message_complete(http_parser* hp);
    static int on_chunk_header(http_parser* hp);
    static int on_chunk_complete(http_parser* hp);

    static http_parser_settings http_setting;
};

}

#endif //SPS_PARSER_HPP

#ifndef SPS_HTTP_PARSER_HPP
#define SPS_HTTP_PARSER_HPP

#include <http_parser.h>

#include <list>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <net/sps_net_io.hpp>
#include <app/url/sps_url.hpp>

#define CRCN "\r\n"

namespace sps {

enum HttpType {
    REQUEST,
    RESPONSE,
    BOTH     // 这里只有测试使用
};

class HttpResponse : public Response {
 public:
    int   content_length = -1;
    int   status_code    = -1;
    bool  chunked        = false;
    std::list<RequestHeader> headers;
};

typedef std::shared_ptr<HttpResponse> PHttpResponse;

class HttpParserContext {
 public:
    PRequestUrl        req;
    PHttpResponse      res;

 public:
    http_parser              http;

    std::list<RequestHeader> headers;
    std::string              url;
    std::string              body;

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
    int parse_request();
    int parse_response();
};

typedef std::shared_ptr<HttpParserContext> PHttpParserContext;

class HttpParser {
 public:
    HttpParser(int max_header = 1024);
    int parse_header(PIReader io, HttpType ht = BOTH);
    int parse_header(const char* buf, int len, HttpType ht = BOTH);

 public:
    PHttpParserContext get_ctx();
    PHttpResponse      get_response();
    PRequestUrl        get_request();

 private:
    int max_header;
    std::unique_ptr<char[]> buf;
    int buf_read;

    RequestHeader      head;
    PHttpParserContext ctx;

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

#endif  // SPS_HTTP_PARSER_HPP

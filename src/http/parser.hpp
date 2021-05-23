#ifndef SPS_PARSER_HPP
#define SPS_PARSER_HPP

#include <http_parser.h>

#include <memory>
#include <list>
#include <string>

#include "net/io.hpp"

namespace sps {

enum HttpType {
    REQUEST,
    RESPONSE,
    BOTH
};

class HttpHeader {
 public:
    std::string key;
    std::string value;
};

class HttpParserResult {
 public:
    int http_status = -1;
    std::string url;
    std::string body;
    std::list<HttpHeader> headers;
};

class HttpParser {
 public:
    HttpParser(int max_header = 1024);
    int parse_header(PIReader io, HttpType ht);
    int parse_header(const char* buf, int len, HttpType ht);

 public:
    const HttpParserResult& get_result();

 private:
    // PIReader io;
    http_parser http;
    HttpType http_type;
    HttpParserResult result;
    int max_header;
    std::unique_ptr<char[]> buf;
    int buf_read;
    HttpHeader head;

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

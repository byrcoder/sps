/*****************************************************************************
MIT License
Copyright (c) 2021 byrcoder

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*****************************************************************************/

#ifndef SPS_HTTP_PARSER_HPP
#define SPS_HTTP_PARSER_HPP

#include <http_parser.h>

#include <list>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <sps_io.hpp>
#include <sps_url.hpp>

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
    explicit HttpParser(int max_header = 2048);
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

}  // namespace sps

#endif  // SPS_HTTP_PARSER_HPP

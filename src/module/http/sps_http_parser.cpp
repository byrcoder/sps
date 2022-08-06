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

#include <sps_http_parser.hpp>
#include <log/sps_log.hpp>

namespace sps {

bool HttpResponse::success() {
    return status_code == 200;
}

std::string HttpResponse::error() {
    return std::to_string(status_code);
}

// http parse的结果
int HttpParserContext::content_length() {
    return http.content_length;
}

bool HttpParserContext::is_chunked() {
    return contains("Transfer-Encoding", (std::string*) nullptr);
}

int HttpParserContext::status_code() {
    return http.status_code;
}

const char* HttpParserContext::method() {
    return http_method_str(static_cast<http_method>(http.method));
}

bool HttpParserContext::contains(const std::string& key, std::string* value) {
    for (auto& h : headers) {
        if (h.key == key) {
            if (value) *value = h.value;
            return true;
        }
    }
    return false;
}

bool HttpParserContext::contains(const std::string& key,
                                 std::vector<std::string>* vs) {
    if (!vs) return contains(key, (std::string*) nullptr);

    for (auto& h : headers) {
        if (h.key == key) {
            (*vs).push_back(h.value);
        }
    }
    return vs->empty();
}

int HttpParserContext::parse_request() {
    std::string host;
    req = std::make_shared<RequestUrl>();
    contains("Host", &host);

    std::string full_url;

    if (!host.empty()) {
        full_url = "http://" + host + url;
    } else {
        full_url = "http:/" + url;
    }

    sp_debug("=======parse url:%s=======", full_url.c_str());

    req->headers = headers;
    req->method  = method();
    return req->parse_url(full_url);
}

int HttpParserContext::parse_response() {
    res = std::make_shared<HttpResponse>();

    res->status_code    = http.status_code;
    res->content_length = http.content_length;
    res->chunked        = http.uses_transfer_encoding;
    res->headers        = headers;

    sp_debug("header size:%lu, status_code:%d, cl:%llu, chunked:%d",
             headers.size(), http.status_code,
             http.content_length, http.uses_transfer_encoding);

    return SUCCESS;
}

http_parser_settings HttpParser::http_setting = {
        .on_message_begin = HttpParser::on_message_begin,
        .on_url           = HttpParser::on_url,
        .on_status        = HttpParser::on_status,
        .on_header_field  = HttpParser::on_header_field,
        .on_header_value  = HttpParser::on_header_value,

        .on_headers_complete = HttpParser::on_headers_complete,
        .on_body             = HttpParser::on_body,
        .on_message_complete = HttpParser::on_message_complete,
        .on_chunk_header     = HttpParser::on_chunk_header,
        .on_chunk_complete   = HttpParser::on_chunk_complete
};

HttpParser::HttpParser(int max_header) {
    this->max_header = max_header;
    buf              = std::unique_ptr<char[]>(new char[max_header]);
    buf_read         = 0;
    ctx              = std::make_shared<HttpParserContext>();
    ctx->http.data   = this;
}

int HttpParser::parse_header(PIReader io, HttpType ht) {
    int state = 4;  // state 0 - 4 from x\r\n\r\n
    error_t ret = SUCCESS;

    do {
        int left = max_header - buf_read;

        if (state > left) {
            sp_error("state %d, left %d, %.*s.", state, left, buf_read, buf.get());
            return -1;
        }

        if ((ret = io->read_fully(buf.get() + buf_read, state,nullptr)) != SUCCESS) {
            sp_debug("failed read ret:%d", ret);
            return ret > 0 ? -ret : ret;
        }
        buf_read += state;

        char *p = buf.get() + (buf_read-1);

        if (*p != '\n' && *p != '\r') {
            state = 4;
        } else if (*p == '\r') {
            if (*(p-1) == '\n' && *(p-2) == '\r') {
                state = 1;
            } else {
                state = 3;
            }
        } else {
            if (*(p-1) != '\r') {
                state = 4;
            }

            if (*(p-2) != '\n' && *(p-3) != '\r')  {
                state = 2;
            } else {
                break;
            }
        }
    } while (true);

    if (ht == HttpType::BOTH && (buf[0] == 'H' && buf[1] == 'T'
                && buf[2] == 'T' && buf[3] == 'P'))  {
        ht = HttpType::REQUEST;
    }

    sp_debug("http head: %s", buf.get());
    return parse_header(buf.get(), buf_read, ht);
}

int HttpParser::parse_header(const char *b, int len, HttpType ht) {
    http_parser_init(&ctx->http, (http_parser_type) ht);

    size_t parsed = http_parser_execute(&ctx->http, &http_setting, b, len);

    if (parsed == 0) {
        sp_error("parse execute zero %.*s", len, b);
    }

    if ((ht == HttpType::REQUEST || ht == HttpType::BOTH) && parsed >= 0)  {
        ctx->parse_request();
    } else if ((ht == HttpType::RESPONSE || ht == HttpType::BOTH)
            && parsed >= 0) {
        ctx->parse_response();
    }

    sp_debug("parsed: %zu, ht:%u", parsed, ht);
    return parsed;
}

PHttpParserContext HttpParser::get_ctx() {
    return ctx;
}

PHttpResponse HttpParser::get_response() {
    return ctx->res;
}

PRequestUrl HttpParser::get_request() {
    return ctx->req;
}

int HttpParser::on_message_begin(http_parser* ) {
    return SUCCESS;
}

int HttpParser::on_url(http_parser* hp, const char *at, size_t length) {
    auto p = static_cast<HttpParser *>(hp->data);

    p->ctx->url = std::string(at, length);
    sp_debug("url: %s", p->ctx->url.c_str());

    return SUCCESS;
}

int HttpParser::on_status(http_parser* hp, const char *at, size_t length) {
    // auto p = static_cast<HttpParser *>(hp->data);
    return SUCCESS;
}

int HttpParser::on_header_field(http_parser* hp, const char *at,
                                size_t length) {
    auto p = static_cast<HttpParser *>(hp->data);

    p->head.key = std::string(at, length);
    sp_debug("key: %s", p->head.key.c_str());

    return SUCCESS;
}

int HttpParser::on_header_value(http_parser* hp, const char *at,
                                size_t length) {
    auto p = static_cast<HttpParser *>(hp->data);

    p->head.value = std::string(at, length);
    sp_debug("header %s->%s", p->head.key.c_str(), p->head.value.c_str());

    p->ctx->headers.push_back(p->head);

    return SUCCESS;
}

int HttpParser::on_headers_complete(http_parser* ) {
    return SUCCESS;
}

int HttpParser::on_body(http_parser* hp, const char *at, size_t length) {
    auto p = static_cast<HttpParser *>(hp->data);

    p->ctx->body = std::string(at, length);
    sp_debug("body: %s", p->ctx->body.c_str());

    return SUCCESS;
}

int HttpParser::on_message_complete(http_parser* ) {
    return SUCCESS;
}

int HttpParser::on_chunk_header(http_parser* ) {
    return SUCCESS;
}

int HttpParser::on_chunk_complete(http_parser* ) {
    return SUCCESS;
}

}  // namespace sps

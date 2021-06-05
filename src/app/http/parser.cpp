#include <app/http/parser.hpp>
#include <log/logger.hpp>

namespace sps {

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

bool HttpParserContext::contains(const std::string& key, std::vector<std::string>* vs) {
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

    sp_info("=======parse url:%s=======", full_url.c_str());

    req->headers = headers;
    return req->parse_url(full_url);
}

int HttpParserContext::parse_response() {
    res = std::make_shared<HttpResponse>();

    res->status_code    = http.status_code;
    res->content_length = http.content_length;
    res->chunked        = http.uses_transfer_encoding;
    res->headers        = headers;

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
    int state = 4; // state 0 - 4 from x\r\n\r\n
    error_t ret = SUCCESS;

    do {
        int left = max_header - buf_read;

        if (state > left) {
            return -1;
        }

        if ((ret = io->read_fully(buf.get() + buf_read, state, nullptr)) != SUCCESS) {
            return ret > 0 ? -ret : ret;
        }
        sp_info("read ret:%d", ret);

        buf_read += state;

        char *p = buf.get() + (buf_read-1);

        if (*p != '\n' && *p != '\r') {
            state = 4;
        } else if (*p == '\r') {
            if (*(p-1) == '\n' && *(p-2) == '\r') state = 1;
            else state = 3;
        } else {
            if (*(p-1) != '\r') state = 4;
            if (*(p-2) != '\n' && *(p-3) != '\r') state = 2;
            else break;
        }
    } while(true);

    if (ht == HttpType::BOTH &&
            (buf[0] == 'H' && buf[1] == 'T' && buf[2] == 'T' && buf[3] == 'P'))  {
        ht = HttpType::REQUEST;
    }

    // sp_info("http head: %s", buf.get());
    return parse_header(buf.get(), buf_read, ht);
}

int HttpParser::parse_header(const char *b, int len, HttpType ht) {
    http_parser_init(&ctx->http, (http_parser_type) ht);

    size_t parsed = http_parser_execute(&ctx->http, &http_setting, b, len);

    sp_info("parsed: %zu, ht:%u", parsed, ht);

    if ((ht == HttpType::REQUEST || ht == HttpType::BOTH)  && parsed >= 0) ctx->parse_request();
    else if ((ht == HttpType::RESPONSE || ht == HttpType::BOTH) && parsed >= 0) ctx->parse_response();

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
    sp_info("url: %s", p->ctx->url.c_str());

    return SUCCESS;
}

int HttpParser::on_status(http_parser* hp, const char *at, size_t length) {
    auto p = static_cast<HttpParser *>(hp->data);

    sp_info("status: %lu, %s", length, at);
    // p->ctx->http_status = atoi(std::string(at, length).c_str());  // 这里不是http status

    return SUCCESS;
}

int HttpParser::on_header_field(http_parser* hp, const char *at, size_t length) {
    auto p = static_cast<HttpParser *>(hp->data);

    p->head.key   = std::string(at, length);
    // sp_info("key: %s", p->head.key.c_str());

    return SUCCESS;
}

int HttpParser::on_header_value(http_parser* hp, const char *at, size_t length) {
    auto p = static_cast<HttpParser *>(hp->data);

    p->head.value = std::string(at, length);
    // sp_info("value: %s", p->head.value.c_str());

    p->ctx->headers.push_back(p->head);

    return SUCCESS;
}

int HttpParser::on_headers_complete(http_parser* ) {
    return SUCCESS;
}

int HttpParser::on_body(http_parser* hp, const char *at, size_t length) {
    auto p = static_cast<HttpParser *>(hp->data);

    p->ctx->body = std::string(at, length);
    sp_info("body: %s", p->ctx->body.c_str());

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

}
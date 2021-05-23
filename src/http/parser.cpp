#include <http/parser.hpp>
#include <log/logger.hpp>

namespace sps {

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
    ctx              = std::shared_ptr<HttpParserContext>();
    ctx->http.data    = this;
    http_type        = BOTH;

}

int HttpParser::parse_header(PIReader io, HttpType ht) {
    int state = 4;

    do {
        int left = max_header - buf_read;
        if (state > left) {
            return -1;
        }

        if (io->read_fully(buf.get(), state, nullptr) != SUCCESS) {
            return -1;
        }

        buf_read += state;

        if (buf[buf_read-1] != '\n' && buf[buf_read-1] != '\r') {
            state = 4;
        } else if (buf[buf_read] == '\r') {
            if (state == 2)  // \r\n\r
                state = 1;
            else // !\n\r
                state = 3;
        } else {
            if (state == 1) // \r\n\r\n
                break;
            else if (state == 3)
                state = 2;
            else
                state = 4;
        }
    } while(true);

    return parse_header(buf.get(), buf_read, ht);
}

int HttpParser::parse_header(const char *b, int len, HttpType ht) {
    this->http_type = ht;
    http_parser_init(&ctx->http, (http_parser_type) ht);

    size_t parsed = http_parser_execute(&ctx->http, &http_setting, b, len);
    return parsed;
}

std::shared_ptr<HttpParserContext> HttpParser::get_ctx() {
    return ctx;
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

    sp_info("status: %s, %d", at, length);
    p->ctx->http_status = atoi(std::string(at, length).c_str());

    return SUCCESS;
}

int HttpParser::on_header_field(http_parser* hp, const char *at, size_t length) {
    auto p = static_cast<HttpParser *>(hp->data);

    p->head.key   = std::string(at, length);
    sp_info("key: %s", p->head.key.c_str());

    return SUCCESS;
}

int HttpParser::on_header_value(http_parser* hp, const char *at, size_t length) {
    auto p = static_cast<HttpParser *>(hp->data);

    p->head.value = std::string(at, length);
    sp_info("value: %s", p->head.value.c_str());

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
#include <http/parser.hpp>
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

    int HttpParserContext::parse_url() {
        contains("Host", &host);

        struct http_parser_url u;

        if (url.empty()) return SUCCESS;

        std::string url = this->url;

        if (url[0] == '/' && !host.empty()) {
            url = "http://" + host + "/" + url;
        }

        sp_info("=======parse url:%s", url.c_str());

        if (http_parser_parse_url(url.c_str(), url.size(), 0, &u) != 0) {
           sp_error("parser url failed:%s", url.c_str());
           return -1;
        }

        if (u.field_set & (1 << UF_SCHEMA))
            schema = url.substr(u.field_data[UF_SCHEMA].off, u.field_data[UF_SCHEMA].len);

        if (u.field_set & (1 << UF_PORT)) port = u.port;
        else                              port = 80;

        if (u.field_set & (1 << UF_HOST))
            host = url.substr(u.field_data[UF_HOST].off, u.field_data[UF_HOST].len);

        if (u.field_set & (1 << UF_PATH))
            path = url.substr(u.field_data[UF_PATH].off, u.field_data[UF_PATH].len);

        if (u.field_set & (1 << UF_QUERY))
            params = url.substr(u.field_data[UF_QUERY].off, u.field_data[UF_QUERY].len);

        auto off_pre = 0;

        do {
            auto off_key   = params.find_first_of('=', off_pre);
            if (off_key == std::string::npos) break;

            std::string key = params.substr(off_pre, off_key - off_pre), value;
            off_pre         = off_key + 1;
            auto off_value  = params.find_first_of('&', off_pre);

            if (off_value == std::string::npos) {
                value   =  params.substr(off_pre);
                pp[key] = value;
                sp_info("=:%u, &:%u, [%s]=[%s], final:%u", off_key, -1, key.c_str(), value.c_str(), off_pre);
                break;
            }

            value = params.substr(off_pre, off_value - off_pre);
            off_pre = off_value + 1;

            sp_info("=:%u, &:%u, [%s]=[%s], final:%u", off_key, off_value, key.c_str(), value.c_str(), off_pre);

        } while(true);

        sp_info("[%s] [%s:%d] [%s] [%s]",
                schema.c_str(), host.c_str(), port, path.c_str(), params.c_str());

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
    http_type        = BOTH;

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

    sp_trace("http head: %s", buf.get());
    return parse_header(buf.get(), buf_read, ht);
}

int HttpParser::parse_header(const char *b, int len, HttpType ht) {
    this->http_type = ht;
    http_parser_init(&ctx->http, (http_parser_type) ht);

    size_t parsed = http_parser_execute(&ctx->http, &http_setting, b, len);

    sp_info("parsed: %u, ht:%d", parsed, ht);

    if ((ht == HttpType::REQUEST || ht == HttpType::BOTH)  && parsed >= 0) ctx->parse_url();

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

    sp_info("status: %s,  %lu", at, length);
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
#include <app/url/url.hpp>

#include <http_parser.h>

#include <log/log_logger.hpp>

namespace sps {

RequestHeader::RequestHeader() {}

RequestHeader::RequestHeader(const std::string &key, const std::string &value) {
    this->key   = key;
    this->value = value;
}

error_t RequestUrl::from(const std::string &url, std::shared_ptr<RequestUrl> &req) {
    req = std::make_shared<RequestUrl>();
    return req->parse_url(url);
}

error_t RequestUrl::parse_url(const std::string& url) {
    struct http_parser_url u;

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
            pp.push_back(RequestHeader(key, value));
            sp_debug("%lu, &:%u, [%s]=[%s], final:%u", off_key, -1, key.c_str(), value.c_str(), off_pre);
            break;
        }

        value = params.substr(off_pre, off_value - off_pre);
        off_pre = off_value + 1;

        sp_debug("%lu, &:%lu, [%s]=[%s], final:%u", off_key, off_value, key.c_str(), value.c_str(), off_pre);
        pp.push_back(RequestParam(key, value));

    } while(true);

    auto n = path.rfind(".");
    if (n != std::string::npos) {
        ext = path.substr(n+1);
    }

    if (method.empty()) {
        method = "GET";
    }

    this->url = path + (params.empty() ? "" : "?" + params);

    sp_info("url:%s -> [%s] [%s:%d] [%s] [%s] [%s]",
            url.c_str(), schema.c_str(), host.c_str(), port, path.c_str(), ext.c_str(), params.c_str());


    return SUCCESS;
}

const char* RequestUrl::get_schema() {
    return schema.c_str();
}

const char* RequestUrl::get_url() {
    return url.c_str();
}

const char* RequestUrl::get_host() {
    return host.c_str();
}

int RequestUrl::get_port() {
    return port;
}

const char* RequestUrl::get_path() {
    return path.c_str();
}

const char* RequestUrl::get_params() {
    return params.c_str();
}

const char* RequestUrl::get_method() {
    return method.c_str();
}

const char * RequestUrl::get_ext() {
    return ext.c_str();
}


std::string RequestUrl::get_header(const std::string& key) {
    for (auto& h : headers) {
        if (h.key == key) return h.value;
    }
    return "";
}

void RequestUrl::erase_header(const std::string &key) {
    for (auto it = headers.begin(); it != headers.end(); ++it) {
        if (it->key == key) {
            headers.erase(it);
            return;
        }
    }
}

std::string RequestUrl::get_param(const std::string& key) {
    for (auto& h : pp) {
        if (h.key == key) return h.value;
    }
    return "";
}

utime_t RequestUrl::get_timeout() {
    return tm;
}

std::string RequestUrl::get_ip() {
    return ip;
}

}

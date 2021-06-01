#include <sstream>

#include <app/http/client.hpp>
#include <st/net/tcp.hpp>
#include <log/logger.hpp>

namespace sps {

string http_request(const string& method, const string& url, const string& host,
                    const string* data, map<string, string>* headers) {
    stringstream ss;
    ss  << method << " " << url << " HTTP/1.1" << CRCN
        << "Host: " << host << CRCN;

    if (headers) {
        for (auto& h : *headers)  ss << h.first << ": " << h.second << CRCN;
    }

    ss << "Accept: */" << CRCN;

    if (data) {
        ss << "Content-Length: " << data->size() << CRCN;
    } else if (method != "GET" && method != "HEAD") {
        ss << "Content-Length: 0" << CRCN;
    }

    ss << CRCN;

    return ss.str();
}

error_t HttpClient::init(const string &host, int port, utime_t to, const string *ip) {
    this->host    = host;
    this->port    = port;
    this->timeout = to;

    if (!ip) this->ip = host;
    else     this->ip = *ip;

    return init_io();
}

error_t HttpClient::get(const string &url, map<string, string> *headers) {
    error_t ret  = SUCCESS;
    string  req  = http_request("GET", url, host, nullptr, headers);

    sp_trace("Request %s.", req.c_str());
    if ((ret = io->write((char*) req.c_str(), req.size())) != SUCCESS) {
        sp_error("Failed write: %s, %d", req.c_str(), ret);
        return ret;
    }

    HttpParser parser;
    if ((ret = parser.parse_header(io, HttpType::RESPONSE)) <= 0) {
        sp_error("Failed parser: %s, %d", req.c_str(), ret);
        return ret;
    }
    ctx = parser.get_ctx();
    return SUCCESS;
}

error_t HttpClient::post(const std::string &url, const std::string &data, map<string, string> *headers) {
    error_t ret  = SUCCESS;
    string  req  = http_request("POST", url, host, nullptr, headers);

    if ((ret = io->write((char*) req.c_str(), req.size())) != SUCCESS) {
        return ret;
    }

    HttpParser parser;
    if ((ret = parser.parse_header(io, HttpType::RESPONSE)) <= 0) {
        return ret;
    }
    ctx = parser.get_ctx();
    return SUCCESS;
}

PIReaderWriter HttpClient::get_io() {
    return io;
}

int HttpClient::status_code() {
    if (ctx) return ctx->status_code();
    return HTTP_STATUS_UNKNOWN;
}

std::shared_ptr<HttpClient> HttpClient::create_http_create() {
    return make_shared<StHttpClient>();
}

// TODO: fixme 这里暴露了st的设计
error_t StHttpClient::init_io() {
    st_netfd_t fd;
    auto ret = st_tcp_connect(ip, port, timeout, &fd);

    if (ret != SUCCESS) return ret;

    io = std::make_shared<StTcpSocket>(fd);
    return ret;
}

}
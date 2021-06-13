#include <app/url/url_http.hpp>

#include <app/http/http_parser.hpp>

#include <net/net_socket.hpp>
#include <log/log_logger.hpp>
#include <string>
#include <sstream>

namespace sps {

std::string http_request(const std::string& method, const std::string& url, const std::string& host,
                    const std::string& data, std::list<RequestHeader>* headers) {
    std::stringstream ss;
    ss  << method << " " << url << " HTTP/1.1" << CRCN
        << "Host: " << host << CRCN;

    if (headers) {
        for (auto& h : *headers)
            if (h.key != "Host")
                ss << h.key << ": " << h.value << CRCN;
    }

    ss << "Accept: */" << CRCN;
    ss << "Content-Length: " << data.size() << CRCN;
    ss << CRCN;

    return ss.str();
}

// TODO: FIX ME
error_t HttpProtocol::open(PRequestUrl url, Transport tp) {
    error_t        ret   = SUCCESS;
    std::string    ip    = (url->get_ip().empty()) ? (url->get_host()) : (url->get_ip());
    Transport      p     = (tp == Transport::DEFAULT ? Transport::TCP : tp);

    auto socket = SingleInstance<ClientSocketFactory>::get_instance().create_ss(
                    p, ip, url->get_port(), url->get_timeout());

    if (!socket) {
        sp_error("failed connect %s:%d, type: %d",
                 ip.c_str(), url->get_port(), p);
        return ERROR_HTTP_SOCKET_CONNECT;
    }

    auto req = http_request(url->method, url->url, url->host, "", &url->headers);

    if ((ret = socket->write((char*) req.c_str(), req.size())) != SUCCESS) {
        sp_error("failed write to %s:%d, ret: %d -> %s.", ip.c_str(),
                url->get_port(), ret, req.c_str());
        return ret;
    }

    sp_trace("success open %s:%d -> request %s.", ip.c_str(), url->get_port(), req.c_str());

    HttpParser parser;
    if ((ret = parser.parse_header(socket, HttpType::RESPONSE)) <= 0) {
        sp_error("Failed parser: %d", ret);
        return ret;
    }

    PHttpResponse http_rsp = rsp = parser.get_response();

    init(socket, ip, url->get_port());

    return SUCCESS;
}

error_t HttpProtocol::read(void *buf, size_t size, size_t& nr) {
    if (eof()) {
        return ERROR_HTTP_RES_EOF;
    }

    nr = 0;
    error_t ret = get_io()->read(buf, size, nr);
    nread += nr;

    return ret;
}

bool HttpProtocol::eof() {
    return rsp->content_length <= nread;
}

PResponse HttpProtocol::response() {
    return rsp;
}

HttpProtocolFactory::HttpProtocolFactory() : IUrlProtocolFactory("http", DEFAULT) {

}

PIURLProtocol HttpProtocolFactory::create(PRequestUrl url) {
    return std::make_shared<HttpProtocol>();
}

}
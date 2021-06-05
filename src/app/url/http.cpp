#include <app/url/http.hpp>

#include <app/http/parser.hpp>

#include <net/socket.hpp>
#include <log/logger.hpp>
#include <string>
#include <sstream>

namespace sps {

std::string http_request(const std::string& method, const std::string& url, const std::string& host,
                    const std::string& data, std::list<RequestHeader>* headers) {
    std::stringstream ss;
    ss  << method << " " << url << " HTTP/1.1" << CRCN
        << "Host: " << host << CRCN;

    if (headers) {
        for (auto& h : *headers)  ss << h.key << ": " << h.value << CRCN;
    }

    ss << "Accept: */" << CRCN;
    ss << "Content-Length: " << data.size() << CRCN;
    ss << CRCN;

    return ss.str();
}

// TODO: FIX ME
error_t HttpProtocol::open(PRequestUrl url, Transport tp) {
    error_t   ret   = SUCCESS;
    auto      ip    = url->get_ip().empty() ? url->get_host() : url->get_host();
    Transport p     = (tp == Transport::DEFAULT ? Transport::TCP : tp);

    auto socket = SingleInstance<ClientSocketFactory>::get_instance().create_ss(
                    p, ip, url->get_port(), url->get_timeout());

    if (!socket) {
        sp_error("Failed connect %s:%d, %d",
                 url->get_ip().c_str(), url->get_port(), p);
        return ERROR_ST_OPEN_SOCKET;
    }

    auto req = http_request(url->method, url->url, url->host, "", &url->headers);

    if ((ret = socket->write((char*) req.c_str(), req.size())) != SUCCESS) {
        sp_error("Failed write: %s, %d", req.c_str(), ret);
        return ret;
    }

    sp_trace("Success Sent Request %s.", req.c_str());

    HttpParser parser;
    if ((ret = parser.parse_header(socket, HttpType::RESPONSE)) <= 0) {
        sp_error("Failed parser: %s, %d", req.c_str(), ret);
        return ret;
    }

    auto resp = parser.get_response();

    if (resp->status_code != HTTP_STATUS_OK) {
        sp_error("Failed get status_code:%s, %d", req.c_str(), resp->status_code);
        return ERROR_ST_OPEN_SOCKET;
    }

    init(socket, ip, url->get_port());

    return SUCCESS;
}

HttpProtocolFactory::HttpProtocolFactory() : IUrlProtocolFactory("http", DEFAULT) {

}

PIURLProtocol HttpProtocolFactory::create(PRequestUrl url) {
    return std::make_shared<HttpProtocol>();
}

}
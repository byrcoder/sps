#include <net/socket.hpp>

#include <co/st/tcp.hpp>
#include <http/client.hpp>
#include <http_parser.h>
#include <sstream>
#include <log/logger.hpp>

namespace sps {

HttpClientSocket::HttpClientSocket(PIReaderWriter io, const std::string &ip, int port) :
    ClientSocket(io, ip, port) {
}

error_t HttpClientSocket::init(int status_code, std::list<Header> *headers,
                               int content_length, bool chunked) {
    this->status_code    = status_code;
    this->content_length = content_length;
    this->chunked        = chunked;
    this->sent_header    = false;

    if (headers) { this->headers = *headers; }

    return SUCCESS;
}

error_t HttpClientSocket::write_header() {
    std::stringstream  ss;
    ss << "HTTP/1.1 " << status_code << " " << http_status_str(
            static_cast<http_status>(status_code)) << CRCN
        << "Server: sps" << CRCN;
    if (chunked) {
        ss << "Transfer-Encoding: chunked" << CRCN;
    } else if (content_length >= 0) {
        ss << "Content-Length: " << content_length << CRCN;
    }

    for (auto& h : headers) {
        ss << h.first << ": " << h.second << CRCN;
    }

    if (content_length == 0) {
        ss << CRCN;
    }

    std::string headers = ss.str();

    sent_header = true;

    return ClientSocket::write((void*) headers.c_str(), headers.size());
}

error_t HttpClientSocket::write(void *buf, size_t size) {
    error_t ret = SUCCESS;

    if (!sent_header && (ret = write_header()) != SUCCESS) {
        sp_error("Failed write head ret:%d", ret);
        return ret;
    }

    if ((ret = ClientSocket::write(buf, size)) != SUCCESS) {
        sp_error("Failed write Buffer ret:%d", ret);
        return ret;
    }
    return ret;
}

IServerSocketFactory& IServerSocketFactory::get_instance() {
    static IServerSocketFactory fc;
    return fc;
}

PIServerSocket IServerSocketFactory::create_ss(Transport transport) {
    switch (transport) {
        case Transport::TCP:
            return std::make_shared<StServerSocket>();
        default:
            return nullptr;
    }
}

}

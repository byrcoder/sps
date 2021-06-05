#include <app/http/http_socket.hpp>

#include <http_parser.h>

#include <sstream>

#include <app/http/http_parser.hpp>
#include <log/log_logger.hpp>

namespace sps {

HttpResponseSocket::HttpResponseSocket(PIReaderWriter rw, const std::string &ip, int p) :
        Socket(std::move(rw), ip, p) {
}

error_t HttpResponseSocket::init(int s_code, std::list<RequestHeader> *hd,
                                 int cl, bool ch) {
    this->status_code    = s_code;
    this->content_length = cl;
    this->chunked        = ch;
    this->sent_header    = false;

    if (hd) { this->headers = *hd; }

    return SUCCESS;
}

error_t HttpResponseSocket::write_header() {
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
        ss << h.key << ": " << h.value << CRCN;
    }
    ss << CRCN;

    std::string http_header = ss.str();

    sent_header = true;

    sp_info ("Http Rsp %s.", http_header.c_str());

    return Socket::write((void*) http_header.c_str(), http_header.size());
}

error_t HttpResponseSocket::write(void *buf, size_t size) {
    error_t ret = SUCCESS;

    if (!sent_header && (ret = write_header()) != SUCCESS) {
        sp_error("Failed write head ret:%d", ret);
        return ret;
    }

    if ((ret = Socket::write(buf, size)) != SUCCESS) {
        sp_error("Failed write Buffer ret:%d", ret);
        return ret;
    }
    return ret;
}

}
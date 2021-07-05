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

#include <sps_url_http.hpp>

#include <sps_http_parser.hpp>
#include <sps_io_socket.hpp>
#include <sps_log.hpp>

#include <algorithm>
#include <list>
#include <memory>
#include <sstream>
#include <string>

namespace sps {

std::string http_request(const std::string& method, const std::string& url,
                         const std::string& host, const std::string& data,
                         std::list<RequestHeader>* headers) {
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

// TODO(byrcoder): FIX ME
error_t HttpUrlProtocol::open(PRequestUrl& url, Transport tp) {
    error_t     ret = SUCCESS;
    std::string ip  = (url->get_ip().empty()) ?
                            (url->get_host()) : (url->get_ip());
    Transport   p   = (tp == Transport::DEFAULT ? Transport::TCP : tp);

    auto socket = SingleInstance<ClientSocketFactory>::get_instance().create_ss(
                    p, ip, url->get_port(), url->get_timeout());

    if (!socket) {
        sp_error("failed connect %s:%d, type: %d",
                 ip.c_str(), url->get_port(), p);
        return ERROR_HTTP_SOCKET_CONNECT;
    }

    auto req = http_request(url->method, url->url, url->host, "",
                            &url->headers);

    if ((ret = socket->write((char*) req.c_str(), req.size())) != SUCCESS) {
        sp_error("failed write to %s:%d, ret: %d -> %s.", ip.c_str(),
                url->get_port(), ret, req.c_str());
        return ret;
    }

    sp_trace("success open %s:%d -> request %s.", ip.c_str(),
              url->get_port(), req.c_str());

    HttpParser parser;
    if ((ret = parser.parse_header(socket, HttpType::RESPONSE)) <= 0) {
        sp_error("Failed parser: %d", ret);
        return ret;
    }

    PHttpResponse http_rsp = rsp = parser.get_response();

    init(socket, ip, url->get_port());

    return SUCCESS;
}

error_t HttpUrlProtocol::read(void *buf, size_t size, size_t& nr) {
    if (eof()) {
        return ERROR_HTTP_RES_EOF;
    }

    if (rsp->chunked) {
        return read_chunked(buf, size, nr);
    }

    // Content-Length default -1, no limited
    if (rsp->content_length >= 0) {
        size = std::min(rsp->content_length-nread, size);
    }

    nr           = 0;
    error_t ret  = get_io()->read(buf, size, nr);

    if (ret == SUCCESS) {
        nread += nr;
        is_eof = nread >= rsp->content_length;
    }

    return ret;
}

error_t HttpUrlProtocol::read_chunked(void *buf, size_t size, size_t& nread) {
    error_t ret = SUCCESS;
    nread       = 0;

    // nb_chunked_left size
    if (nb_chunked_left == 0) {
        ret = read_chunked_length();
    }

    if (ret != SUCCESS) {
        return ret;
    }

    ret = read_chunked_data(buf, size, nread);

    return ret;
}

error_t HttpUrlProtocol::read_chunked_length() {
    error_t    ret = SUCCESS;
    size_t     nr  = 0;
    char       tmp_buf[34];

    int i = 0;
    do {
        ret = get_io()->read(tmp_buf+i, 1, nr);
        ++i;
    } while (ret == SUCCESS && tmp_buf[i-1] != '\n' && i < sizeof(tmp_buf)-1);

    if (ret != SUCCESS) {
        return ret;
    }

    if (tmp_buf[i-1] != '\n') {
        return ERROR_HTTP_CHUNKED_LENGTH_LARGE;
    }

    if (i < 2 || tmp_buf[i-2] != '\r') {
        sp_error("invalid read chunked length i:%d", i);
        return ERROR_HTTP_CHUNKED_INVALID;
    }

    tmp_buf[i-2] = '\0';  // \r

    nb_chunked_left = nb_chunked_size = std::strtoul(tmp_buf, nullptr, 16);

    sp_debug("read chunked length %lu", nb_chunked_size);
    return ret;  // SUCCESS
}

error_t HttpUrlProtocol::read_chunked_data(void *buf, size_t size,
                                           size_t& nread) {
    size         = std::min(nb_chunked_left, size);
    error_t ret  =  SUCCESS;

    if (nb_chunked_size != 0) {
        ret = get_io()->read_fully(buf, size, (ssize_t *) &nread);
    }

    nb_chunked_left -= nread;  // ignore ret

    sp_debug("read chunked data ret:%d, nread:%lu", ret, nread);

    if (ret == SUCCESS && nb_chunked_left == 0) {
        char tail[2];
        ret = get_io()->read_fully(tail, 2, nullptr);

        if (ret == SUCCESS && (tail[0] != '\r' || tail[1] != '\n')) {
            sp_error("invalid read http chunked data");
            ret = ERROR_HTTP_CHUNKED_INVALID;
        }
    }

    if (ret == SUCCESS && nb_chunked_size == 0) {
        is_eof = true;
    }
    return ret;
}

bool HttpUrlProtocol::eof() {
    return is_eof;
}

PResponse HttpUrlProtocol::response() {
    return rsp;
}

HttpURLProtocolFactory::HttpURLProtocolFactory()
    : IURLProtocolFactory("http", DEFAULT) {
}

PIURLProtocol HttpURLProtocolFactory::create(PRequestUrl url) {
    return std::make_shared<HttpUrlProtocol>();
}

}  // namespace sps

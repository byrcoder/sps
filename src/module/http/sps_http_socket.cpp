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

#include <sps_http_socket.hpp>

#include <http_parser.h>

#include <sstream>

#include <sps_http_parser.hpp>
#include <sps_log.hpp>

namespace sps {

HttpRequestSocket::HttpRequestSocket(PIReaderWriter rw,
                                     const std::string &ip,
                                     int port,
                                     bool chunked,
                                     int cl) :
        Socket(std::move(rw), ip, port) {
    this->chunked        = chunked;
    this->content_length = cl;
}

error_t HttpRequestSocket::read(void* buf, size_t size, size_t& nr) {
    if (eof()) {
        return ERROR_HTTP_RES_EOF;
    }

    if (chunked) {
        return read_chunked(buf, size, nr);
    }

    // Content-Length default -1, no limited
    if (content_length >= 0) {
        size = std::min(content_length-nread, size);
    }

    nr = 0;
    error_t ret  = get_io()->read(buf, size, nr);

    if (ret == SUCCESS) {
        nread += nr;

        if (content_length >= 0) {
            is_eof = nread >= content_length;
        }
    }

    return ret;
}

bool HttpRequestSocket::eof() {
    return is_eof;
}

error_t HttpRequestSocket::read_chunked(void* buf, size_t size, size_t& nread) {
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

error_t HttpRequestSocket::read_chunked_length() {
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

error_t HttpRequestSocket::read_chunked_data(void* buf, size_t size, size_t& nread) {
    size         = std::min(nb_chunked_left, size);
    error_t ret  =  SUCCESS;

    if (nb_chunked_size != 0) {
        ret = get_io()->read(buf, size, nread);
    }

    nb_chunked_left -= nread;  // ignore ret

    sp_debug("read chunked data nb_chunked_size: %lu, "
            "nb_chunked_left: %lu, ret: %d, nread: %lu",
            nb_chunked_size, nb_chunked_left, ret, nread);

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

HttpResponseSocket::HttpResponseSocket(PIReaderWriter rw,
                                       const std::string &ip,
                                       int p) :
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
    error_t  ret           = SUCCESS;
    bool     content_sent  = false;
    bool     has_server    = false;

    std::stringstream  ss;
    ss << "HTTP/1.1 " << status_code << " " << http_status_str(
            static_cast<http_status>(status_code)) << CRCN;
    if (chunked) {
        ss << "Transfer-Encoding: chunked" << CRCN;
    } else if (content_length >= 0) {
        ss << "Content-Length: " << content_length << CRCN;
        content_sent = true;
    }

    for (auto& h : headers) {
        if (h.key == "Content-Length" && content_sent) {
            continue;
        }

        if (h.key == "Server") {
            has_server = true;
        }
        ss << h.key << ": " << h.value << CRCN;
    }

    if (!has_server) {
        ss << "Server: sps" << CRCN;
    }
    ss << CRCN;

    std::string http_header = ss.str();
    sent_header = true;

    ret = Socket::write((void*) http_header.c_str(), http_header.size());

    sp_debug("http wrote rsp header ret: %d,  chunked: %d, %s.",
             ret, chunked, http_header.c_str());

    return ret;
}

error_t HttpResponseSocket::write(void *buf, size_t size) {
    error_t ret = SUCCESS;

    if (!sent_header && (ret = write_header()) != SUCCESS) {
        sp_error("Failed write head ret:%d", ret);
        return ret;
    }

    if (chunked) {
        std::stringstream ss;
        ss << std::hex << size << "\r\n";
        std::string tmp = ss.str();
        if ((ret = Socket::write((void *) tmp.c_str(),
                tmp.size())) != SUCCESS) {
            sp_error("failed write http chunked length size:%lu, %s, ret:%d",
                      size, tmp.c_str(), ret);
            return ret;
        }
        sp_debug("success write http chunked length size: %lu(%x), %s, ret:%d",
                   size, size, tmp.c_str(), ret);
    }

    if (size > 0) {
        if ((ret = Socket::write(buf, size)) != SUCCESS) {
            sp_error("Failed write Buffer ret:%d", ret);
            return ret;
        }
        sp_debug("success write http chunked data size: %lu, %s, ret:%d", size,
                std::string((char*)buf, size).c_str(), ret);
    }

    if (chunked) {
        if ((ret = Socket::write((void*) "\r\n",
                sizeof("\r\n")-1)) != SUCCESS) {
            sp_error("failed write http chunked end Buffer ret:%d", ret);
            return ret;
        }
        sp_debug("success write http chunked end: ret:%d", ret);
    }

    return ret;
}

}  // namespace sps

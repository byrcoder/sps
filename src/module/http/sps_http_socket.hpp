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

#ifndef SPS_HTTP_SOCKET_HPP
#define SPS_HTTP_SOCKET_HPP

#include <memory>
#include <list>
#include <string>
#include <utility>

#include <sps_url.hpp>
#include <sps_io_socket.hpp>

namespace sps {

class HttpRequestSocket : public Socket {
 public:
    HttpRequestSocket(PIReaderWriter rw, const std::string& ip,
                      int port, bool chunked, int content_length);

 public:
    error_t read(void* buf, size_t size, size_t& nread) override;
    bool    eof();

 private:
    error_t read_chunked(void* buf, size_t size, size_t& nread);
    error_t read_chunked_length();
    error_t read_chunked_data(void* buf, size_t size, size_t& nread);

 public:
    bool   chunked;
    int    content_length;
    bool   is_eof          = false;
    size_t nread           = 0;
    size_t nb_chunked_size = 0;
    size_t nb_chunked_left = 0;
};

class HttpResponseSocket : public Socket {
 public:
    HttpResponseSocket(PIReaderWriter rw, const std::string& ip, int port);

 public:
    error_t init(int s_code, std::list<RequestHeader>* hd,
                 int content_len, bool chunked);
    error_t write_header();
    error_t write(void* buf, size_t size) override;

 private:
    int  status_code    = 200;
    bool sent_header    = false;
    int  content_length = 0;
    bool chunked        = false;
    std::list<RequestHeader> headers;
};
typedef std::shared_ptr<HttpResponseSocket> PHttpResponseSocket;

}  // namespace sps

#endif  // SPS_HTTP_SOCKET_HPP

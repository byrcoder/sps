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

#ifndef SPS_IO_URL_HTTP_HPP
#define SPS_IO_URL_HTTP_HPP

#include <map>
#include <string>

#include <sps_http_parser.hpp>
#include <sps_io_url_protocol.hpp>

namespace sps {

class HttpUrlProtocol : public IURLProtocol {
 public:
    HttpUrlProtocol() = default;

 public:
    error_t open(PRequestUrl& url, Transport p) override;

 public:
    error_t read(void* buf, size_t size, size_t& nread) override;

    error_t read_chunked(void* buf, size_t size, size_t& nread);

    error_t read_chunked_length();

    error_t read_chunked_data(void* buf, size_t size, size_t& nread);

 public:
    bool eof();

 public:
    PResponse response() override;

 private:
    PHttpResponse rsp;
    size_t  nread = 0;

    size_t nb_chunked_size = 0;
    size_t nb_chunked_left = 0;

    bool is_eof         = false;
};

class HttpURLProtocolFactory : public IURLProtocolFactory {
 public:
    HttpURLProtocolFactory();

 public:
    PIURLProtocol create(PRequestUrl url);
};

}  // namespace sps

#endif  // SPS_IO_URL_HTTP_HPP

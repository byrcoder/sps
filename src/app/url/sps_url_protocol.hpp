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

#ifndef SPS_URL_PROTOCOL_HPP
#define SPS_URL_PROTOCOL_HPP

#include <app/url/sps_url.hpp>

#include <net/sps_net_socket.hpp>

namespace sps {

class IURLProtocol : public Socket {
 public:
    IURLProtocol() = default;

 public:
    virtual error_t open(PRequestUrl url, Transport p = DEFAULT) = 0;
    error_t open(const std::string url, Transport p);

 public:
    virtual PResponse response() = 0;
};
typedef std::shared_ptr<IURLProtocol> PIURLProtocol;

class IUrlProtocolFactory  {
 public:
    IUrlProtocolFactory(const char* schema, Transport t);

 public:
    ~IUrlProtocolFactory() = default;

 public:
    virtual bool match(PRequestUrl url);
    virtual PIURLProtocol create(PRequestUrl url) = 0;

 public:
    const char* schema;
    Transport   t;
};

typedef std::shared_ptr<IUrlProtocolFactory> PIUrlProtocolFactory;

class UrlProtocol : public Registers<UrlProtocol, PIUrlProtocolFactory> {
 public:
    PIURLProtocol create(PRequestUrl url);
    PIURLProtocol create(const std::string& url);
};

}

#endif  // SPS_URL_PROTOCOL_HPP

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

#ifndef SPS_URL_RTMP_HPP
#define SPS_URL_RTMP_HPP

#include <sps_rtmp_librtmp.hpp>
#include <sps_url_protocol.hpp>

namespace sps {

class RtmpUrlProtocol : public IURLProtocol {
 public:
    explicit RtmpUrlProtocol() = default;

    explicit RtmpUrlProtocol(std::shared_ptr<RtmpHook> hk);

 public:
    error_t open(PRequestUrl& url, Transport p) override;

 public:
    error_t read(void* buf, size_t size, size_t& nread) override;

    error_t read(WrapRtmpPacket& packet);

    error_t write(WrapRtmpPacket& packet);

 public:
    PResponse response() override ;

 private:
    std::shared_ptr<RtmpHook> hk;
};

typedef std::shared_ptr<RtmpUrlProtocol> PRtmpUrlProtocol;

class RtmpURLProtocolFactory : public IURLProtocolFactory {
 public:
    RtmpURLProtocolFactory();

 public:
    PIURLProtocol create(PRequestUrl url) override;
};

}

#endif  // SPS_URL_RTMP_HPP

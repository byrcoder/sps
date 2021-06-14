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

#ifndef SPS_STREAM_DEC_HPP
#define SPS_STREAM_DEC_HPP

#include <app/url/sps_url.hpp>
#include <cache/sps_cache_buffer.hpp>

#include <net/sps_net_io.hpp>

#include <sps_typedef.hpp>
#include <list>
#include <app/url/sps_url_protocol.hpp>

namespace sps {

class IAVInputFormat;

class IAvDemuxer;
typedef std::shared_ptr<IAvDemuxer> PIAvDemuxer;

// 解封装协议
class IAvDemuxer {
 public:
    virtual ~IAvDemuxer() = default;

 public:
    const IAVInputFormat* name();

 public:
    virtual error_t read_header(PIBuffer& buffer)  = 0;
    virtual error_t read_message(PIBuffer& buffer) = 0;
    virtual error_t read_tail(PIBuffer& buffer)    = 0;
    virtual error_t probe(PIBuffer& buffer)        = 0;

 public:
    const IAVInputFormat* fmt;
};

// IAvDemuxer创建方法，是否匹配的方法
class IAVInputFormat {
 public:
    IAVInputFormat(const char*name, const char* ext);
    virtual ~IAVInputFormat() = default;

 public:
    virtual bool match(const char* ext) const;

 public:
    virtual PIAvDemuxer create(PIURLProtocol p) = 0;

 public:
    const char* name;
    const char* ext_name;
};

typedef std::shared_ptr<IAVInputFormat> PIAVInputFormat;

// c++ 没有无反射，每个IAvDeumxer的创建需要实现工厂方法
class AvDemuxerFactory : public Registers<AvDemuxerFactory, PIAVInputFormat> {
 public:
    AvDemuxerFactory() = default;

 public:
    virtual error_t init();

 public:
    // TODO: to impl probe media format
    virtual PIAvDemuxer probe(PIURLProtocol& p, PRequestUrl& url);

 public:
    PIAvDemuxer create(PIURLProtocol& p, PRequestUrl& url);
    PIAvDemuxer create(PIURLProtocol& p, const std::string& url);
};

}

#endif  // SPS_STREAM_DEC_HPP

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

#ifndef SPS_AVFORMAT_DEC_HPP
#define SPS_AVFORMAT_DEC_HPP

#include <list>

#include <sps_cache_buffer.hpp>
#include <sps_io.hpp>
#include <sps_typedef.hpp>
#include <sps_url.hpp>
#include <sps_url_protocol.hpp>

namespace sps {

class IAVInputFormat;

class IAVDemuxer;
typedef std::shared_ptr<IAVDemuxer> PIAVDemuxer;

// stream demuxer for example flv/ts/mp4
class IAVDemuxer {
 public:
    virtual ~IAVDemuxer() = default;

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

// IAVInputFormat is the factory of IAVDemuxer
class IAVInputFormat {
 public:
    IAVInputFormat(const char*name, const char* ext);
    virtual ~IAVInputFormat() = default;

 public:
    virtual bool match(const char* ext) const;

 public:
    virtual PIAVDemuxer create(PIReader p) = 0;

 public:
    const char* name;
    const char* ext;
};

typedef std::shared_ptr<IAVInputFormat> PIAVInputFormat;

// AVDemuxerFactory consists of all IAVInputFormat
class AVDemuxerFactory : public Registers<AVDemuxerFactory, PIAVInputFormat> {
 public:
    AVDemuxerFactory() = default;

 public:
    // TODO: to impl probe media format
    virtual PIAVDemuxer probe(PIReader p, PRequestUrl& url);

 public:
    PIAVDemuxer create(PIReader p, PRequestUrl& url);
    PIAVDemuxer create(PIReader p, const std::string& url);
};

}

#endif  // SPS_AVFORMAT_DEC_HPP

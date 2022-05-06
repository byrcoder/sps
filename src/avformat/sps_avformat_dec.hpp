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
#include <memory>
#include <string>
#include <utility>

#include <sps_avformat_packet.hpp>
#include <sps_io.hpp>
#include <sps_typedef.hpp>
#include <sps_url.hpp>
#include <sps_url_protocol.hpp>

namespace sps {

class IAVInputFormat;

// stream demuxer for example flv/ts/mp4
class IAVDemuxer {
 public:
    virtual ~IAVDemuxer() = default;

 public:
    const IAVInputFormat* name() { return fmt; }

 public:
    virtual error_t read_header(PAVPacket & buffer)  = 0;
    virtual error_t read_packet(PAVPacket& buffer)  = 0;
    virtual error_t read_tail(PAVPacket& buffer)    = 0;
    virtual error_t probe(PAVPacket& buffer)        = 0;
    virtual IAVContext*  get_av_ctx() { return nullptr; }

 public:
    const IAVInputFormat* fmt;
};
typedef std::shared_ptr<IAVDemuxer> PIAVDemuxer;

// IAVInputFormat is the factory of IAVDemuxer
class IAVInputFormat {
 public:
    IAVInputFormat(const char*name, const char* ext);
    virtual ~IAVInputFormat() = default;

 public:
    virtual bool match(const char* ext) const;

 protected:
    virtual PIAVDemuxer _create(PIReader p) = 0;

 public:
    PIAVDemuxer create2(PIReader p);

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
    // TODO(byrcoder): to impl probe media format
    virtual PIAVDemuxer probe(PIReader p, PRequestUrl& url);

 public:
    PIAVDemuxer create(PIReader p, PRequestUrl& url);
    PIAVDemuxer create(PIReader p, const std::string& url);
};

#define AVInputFormat(Name, name, ext) \
    class Name##AVInputFormat : public IAVInputFormat { \
     public: \
        Name##AVInputFormat() : IAVInputFormat(name, ext) { } \
            \
     public: \
        PIAVDemuxer _create(PIReader p) override { \
            return std::make_shared<Name##Demuxer>(std::move(p)); \
        } \
    }; \

}  // namespace sps

#endif  // SPS_AVFORMAT_DEC_HPP

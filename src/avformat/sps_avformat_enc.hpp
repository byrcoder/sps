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

#ifndef SPS_STREAM_ENC_HPP
#define SPS_STREAM_ENC_HPP

#include <sps_cache_buffer.hpp>
#include <sps_io.hpp>
#include <sps_typedef.hpp>
#include <sps_url.hpp>
#include <sps_url_protocol.hpp>
#include "sps_avformat_packet.hpp"

namespace sps {

class IAVOutputFormat;
// stream encoder for example flv/ts/mp4
class IAVMuxer {
 public:
    virtual ~IAVMuxer() = default;

 public:
    const IAVOutputFormat* name();

 public:
    virtual error_t write_header(PSpsAVPacket& buffer)  = 0;
    virtual error_t write_message(PSpsAVPacket& buffer) = 0;
    virtual error_t write_tail(PSpsAVPacket& buffer)    = 0;

 public:
    const IAVOutputFormat* fmt;
};

typedef std::shared_ptr<IAVMuxer> PIAVMuxer;

class IAVOutputFormat {
 public:
    IAVOutputFormat(const char*name, const char* ext);
    virtual ~IAVOutputFormat() = default;

 public:
    virtual bool match(const char* ext) const;

 public:
    virtual PIAVMuxer create(PIWriter pw) const = 0;

 public:
    const char* name;
    const char* ext;
};

typedef std::shared_ptr<IAVOutputFormat> PIAVOutputFormat;

// AVDemuxerFactory consists of all IAVInputFormat
class AVEncoderFactory : public Registers<AVEncoderFactory, PIAVOutputFormat> {
 public:
    AVEncoderFactory() = default;

 public:
    PIAVMuxer create(PIWriter p, PRequestUrl& url);
    PIAVMuxer create(PIWriter p, const std::string& url);
};

}

#endif  // SPS_STREAM_ENC_HPP

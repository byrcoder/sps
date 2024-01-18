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

#ifndef SPS_AVCODEC_PARSER_HPP
#define SPS_AVCODEC_PARSER_HPP

#include <sps_util_typedef.hpp>

#include <sps_avformat_packet.hpp>
#include <list>

namespace sps {

enum AVCodec {
    AVCODEC_H264 = 7,

    AVCODEC_AAC  = 10,

    AVCODEC_H265 = 12
};

class AVCodecContext {
 public:
    AVCodecContext(int64_t dts = -1, int64_t pts = -1, int64_t timebase = 1);
    int64_t dts;
    int64_t pts;
    int64_t timebase;
};

class IAVCodecParser {
 public:
    virtual error_t encode_avc(AVCodecContext* ctx, uint8_t* in_buf,
                               int in_size, std::list<PAVPacket>& pkts) = 0;
};
typedef std::shared_ptr<IAVCodecParser> PIAVCodecParser;

}  // namespace sps

#endif  // SPS_AVCODEC_PARSER_HPP

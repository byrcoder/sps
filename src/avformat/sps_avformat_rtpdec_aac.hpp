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

#ifndef SPS_AVFORMAT_RTPDEC_AAC_HPP
#define SPS_AVFORMAT_RTPDEC_AAC_HPP

#include <sps_avformat_rtpdec.hpp>

#include <sps_avcodec_aac_parser.hpp>

namespace sps {

struct RtpAacAu {
    uint16_t au_size : 13;
    uint16_t au_index : 3;
};

class RtpAacAuHeader {
 public:
    error_t decode(BitContext& bc);

 public:
    int au_headers_length;  // bits
    int nb_au_headers;
    std::list<RtpAacAu> au_lists;
};

class AacRtpDecoder : public IRtpDynamicDecoder {
 public:
    AacRtpDecoder();

 public:
    error_t decode(RtpPayloadHeader& header, BitContext& bc, std::list<PAVPacket>& pkts);

 private:
    AacAVCodecParser aac_parser;
};

}  // namespace sps

#endif  // SPS_AVFORMAT_RTPDEC_AAC_HPP

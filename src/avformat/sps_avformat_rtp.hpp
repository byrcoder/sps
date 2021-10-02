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

#ifndef SPS_AVFORMAT_RTP_HPP
#define SPS_AVFORMAT_RTP_HPP

#include <vector>

#include <sps_avformat_packet.hpp>
#include <sps_io_bits.hpp>

#define RTP_MAX_LENGTH 3000
#define RTP_NALU_MAX_LENGTH 1 * 1024 * 1024

namespace sps {

/**
 * https://datatracker.ietf.org/doc/html/rfc1890
 */
enum RtpPayLoadType {
    RTP_TS = 33,

    RTP_DYNAMIC = 96,
    RTP_DYNAMIC_END = 127,

    RTP_SR = 200,
    RTP_RR = 201,
    RTP_SDES = 202,
    RTP_BYE  = 203,
    RTP_APP  = 204,
};

bool is_rtp_mepgts(int pt);
bool is_rtp_dynamic(int pt);
bool is_rtcp(int pt);


class RtpPayloadHeader {
 public:
    error_t decode(BitContext& bc);

 private:
    error_t decode_extension(BitContext& bc);

 public:
    struct {
        uint32_t version : 2;
        uint32_t padding : 1;
        uint32_t extension : 1;
        uint32_t cc : 4;
        uint32_t mark : 1;
        // http://www.iana.org/assignments/rtp-parameters/rtp-parameters.xhtml
        uint32_t payload_type : 7;
        uint32_t sequence_num : 16;

        uint32_t timestamp;
        uint32_t ssrc;
    } fix_header;

    // number of cc
    std::vector<uint32_t> csrc;

    struct {
        uint16_t define_by_profile;
        uint16_t length;
        std::unique_ptr<uint8_t[]> header_extension;
    } extension_info;
};

}  // namespace sps

#endif  // SPS_AVFORMAT_RTP_HPP

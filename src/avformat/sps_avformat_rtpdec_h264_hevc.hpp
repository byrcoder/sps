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

#ifndef SPS_AVFORMAT_RTPDEC_H264_HPP
#define SPS_AVFORMAT_RTPDEC_H264_HPP

#include <sps_avformat_rtp.hpp>

#include <sps_avcodec_h264_parser.hpp>
#include <sps_avformat_rtpdec.hpp>

namespace sps {

// Table 1.  Summary of NAL unit types and the corresponding packet
//                types
//
//      NAL Unit  Packet    Packet Type Name               Section
//      Type      Type
//      -------------------------------------------------------------
//      0        reserved                                     -
//      1-23     NAL unit  Single NAL unit packet             5.6
//      24       STAP-A    Single-time aggregation packet     5.7.1
//      25       STAP-B    Single-time aggregation packet     5.7.1
//      26       MTAP16    Multi-time aggregation packet      5.7.2
//      27       MTAP24    Multi-time aggregation packet      5.7.2
//      28       FU-A      Fragmentation unit                 5.8
//      29       FU-B      Fragmentation unit                 5.8
//      30-31    reserved                                     -

// Packetization Modes sdp
//Payload Packet    Single NAL    Non-Interleaved    Interleaved
//Type    Type      Unit Mode           Mode             Mode
//-------------------------------------------------------------
//0      reserved      ig               ig               ig
//1-23   NAL unit     yes              yes               no
//24     STAP-A        no              yes               no
//25     STAP-B        no               no              yes
//26     MTAP16        no               no              yes
//27     MTAP24        no               no              yes
//28     FU-A          no              yes              yes
//29     FU-B          no               no              yes
//30-31  reserved      ig               ig               ig
enum H264RtpNaluType {
    ZERO_RESERVED = 0,
    // 1-23 nalu type
    STAP_A = 24,
    STAP_B = 25,
    MTAP16 = 26,
    MTAP24 = 27,
    FU_A   = 28,
    FU_B   = 29,
};

struct FuIndicator {
    uint8_t f : 1;
    uint8_t nri : 2;
    uint8_t tp : 5;
};

struct FuHeader {
    uint8_t s : 1;
    uint8_t e : 1;
    uint8_t r : 1;
    uint8_t tp : 5;
};

class FuDecoder {
 public:
    FuDecoder();
    error_t decode(BitContext& bc);

 public:
    bool is_start();
    bool is_end();

 public:
    FuIndicator indicator;
    FuHeader    header;
};

// h264 rtp decoder
class IRtpH264Decoder {
 public:
    virtual error_t decode(int tp, RtpPayloadHeader& header,
            BitContext& bc, std::list<PAVPacket>& pkts) = 0;
};
typedef std::shared_ptr<IRtpH264Decoder> PIRtpH264Decoder;

// raw nalu decode
class NaluRtpDecoder : public IRtpH264Decoder {
 public:
    error_t decode(int tp, RtpPayloadHeader& header,
            BitContext& bc, std::list<PAVPacket>& pkts) override;
};

// stap-a/b decode
class StapRtpDecoder : public IRtpH264Decoder {
 public:
    error_t decode(int tp, RtpPayloadHeader& header,
                   BitContext& bc, std::list<PAVPacket>& pkts) override;
};

// mtap-a/b decode
class MtapRtpDecoder : public IRtpH264Decoder {
 public:
    error_t decode(int tp, RtpPayloadHeader& header,
                   BitContext& bc, std::list<PAVPacket>& pkts) override;
};

// fu-a/b decode
class FuRtpDecoder : public IRtpH264Decoder {
 public:
    FuRtpDecoder(PCharBuffer& pkt, int64_t rtp_timestamp);
 public:
    error_t decode(int tp, RtpPayloadHeader& header,
                   BitContext& bc, std::list<PAVPacket>& pkts) override;

 private:
    PCharBuffer& pkt;
    int64_t pre_rtp_timestamp;
};

class H264RtpDecoder : public IRtpDynamicDecoder {
 public:
    PIRtpH264Decoder create_decoder(BitContext& bc, uint8_t nal_unit_type);

 public:
    error_t decode(RtpPayloadHeader& header, BitContext& bc, std::list<PAVPacket>& pkts) override;

 private:
    PCharBuffer pkt;
    int64_t rtp_timestamp;
};

}  // namespace sps

#endif  // SPS_AVFORMAT_RTPDEC_H264_HPP

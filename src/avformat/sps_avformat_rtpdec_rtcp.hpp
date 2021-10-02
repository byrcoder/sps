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

#ifndef SPS_AVFORMAT_RTPDEC_RTCP_HPP
#define SPS_AVFORMAT_RTPDEC_RTCP_HPP

#include <sps_avformat_rtpdec.hpp>

namespace sps {

struct RtcpHead {
    uint32_t version : 2;
    uint32_t padding : 1;
    uint32_t cflag : 5;  // rc,sc,subtype
    uint32_t payload_type : 8;
    uint32_t length : 16;
};

class RtcpHeadDecoder {
 public:
    error_t decode(BitContext& bc);
 public:
    RtcpHead header;
};


class IRtcpDecoder {
 public:
    virtual error_t decode(RtcpHead& h, BitContext& bc) = 0;
    virtual std::string debug();
};
typedef std::shared_ptr<IRtcpDecoder> PIRtcpDecoder;

class RtcpSRReportBlock {
 public:
    static const int SIZE = 20;
 public:
    error_t decode(BitContext& bc);

    uint32_t fraction_lost : 8;
    uint32_t cum_number_pkt_lost : 24;  // 24 bit
    uint32_t extend_high_seq_num_recv;
    uint32_t inter_arrival_jitter;
    uint32_t last_sr;
    uint32_t delay_since_last_sr;
};

class RtcpSRDecoder : public IRtcpDecoder {
 public:
    error_t decode(RtcpHead& h, BitContext& bc) override;
    std::string debug() override;

 public:
    uint32_t ssrc;
    uint32_t ntp_ts_msw;
    uint32_t ntp_ts_lsw;
    uint32_t rtp_ts;
    uint32_t send_pkt_count;
    uint32_t send_oct_count;
    std::list<RtcpSRReportBlock> report_blocks;
};

class RtcpRRDecoder : public IRtcpDecoder {
 public:
    error_t decode(RtcpHead& h, BitContext& bc) override;
    std::string debug() override;

 public:
    uint32_t ssrc;
    std::list<RtcpSRReportBlock> report_blocks;
};

// 6.5 SDES: Source Description RTCP Packet
class RtcpSDES {
 public:
    RtcpSDES() = default;
    error_t decode(BitContext& bc);
 public:
    uint32_t ssrc;
    uint8_t  item_type;
    uint8_t  item_length;
    PCharBuffer item_value;
};

class RtcpSDESDecoder : public IRtcpDecoder {
 public:
    error_t decode(RtcpHead& h, BitContext& bc) override;
    std::string debug() override;

 public:
    std::list<RtcpSDES> sdes;
};

class RtcpByteDecoder : public IRtcpDecoder {
 public:
    error_t decode(RtcpHead& h, BitContext& bc) override;
    std::string debug() override;
};

class RtcpAppDecoder : public IRtcpDecoder {
 public:
    error_t decode(RtcpHead& h, BitContext& bc) override;
    std::string debug() override;
};

class RtcpRtpDecoder : public IRtpDecoder {
 public:
    error_t decode(uint8_t* buffer, int size, std::list<PAVPacket>& pkts) override;

    /**
     * payload decode for diff payload type
     * @param pt rtp payload type header
     * @return
     */
    bool match(int pt) override;
    error_t decode(RtpPayloadHeader& header, BitContext& bc, std::list<PAVPacket>& pkts) override;
};

}  // namespace sps

#endif  // SPS_AVFORMAT_RTPDEC_RTCP_HPP

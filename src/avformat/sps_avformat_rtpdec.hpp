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

#ifndef SPS_AVFORMAT_RTPDEC_HPP
#define SPS_AVFORMAT_RTPDEC_HPP

#include <sps_avformat_dec.hpp>
#include <sps_avformat_rtp.hpp>

#include <sps_io_bytes.hpp>

namespace sps {

/**
 * rtp decode base
 */
class IRtpDecoder {
 public:
    /**
     * @param buffer
     * @param size
     * @param pkts result of av packet
     * @return
     */
    virtual error_t decode(uint8_t* buffer, int size, std::list<PAVPacket>& pkts);
    /**
     * payload decode for diff payload type
     * @param pt rtp payload type header
     * @return
     */
    virtual bool match(int pt) = 0;
    virtual error_t decode(RtpPayloadHeader& header, BitContext& bc, std::list<PAVPacket>& pkts) = 0;
};

typedef std::shared_ptr<IRtpDecoder> PIRtpDecoder;

/**
 * dynamic rtp decode payload 96
 */
class IRtpDynamicDecoder : public IRtpDecoder {
 public:
    bool match(int pt) override;
    error_t decode(RtpPayloadHeader& header, BitContext& bc, std::list<PAVPacket>& pkts) override = 0;
};

typedef std::shared_ptr<IRtpDynamicDecoder> PIRtpDynamicDecoder;

class RtpDemuxer : public IAVDemuxer {
 public:
    explicit RtpDemuxer(PIReader rd);

 public:
    error_t init_codec(int codec_id);
    error_t init_rtp_decode(int payload_type);

 public:
    error_t read_header(PAVPacket & buffer)  override;
    error_t read_packet(PAVPacket& buffer)   override;
    error_t read_tail(PAVPacket& buffer)    override;
    error_t probe(PAVPacket& buffer)        override;

 private:
    PAVBuffer buf;
    PBytesReader rd;
    PIRtpDecoder rtp_decoder;
    PIRtpDecoder rtcp_decoder;

    std::list<PAVPacket> pkts;
};

AVInputFormat(Rtp, "rtp", "rtp");

}  // namespace sps

#endif  // SPS_AVFORMAT_RTPDEC_HPP

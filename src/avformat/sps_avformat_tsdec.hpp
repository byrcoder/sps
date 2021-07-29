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

#ifndef SPS_AVFORMAT_TSDEC_HPP
#define SPS_AVFORMAT_TSDEC_HPP

#include <memory>

#include <sps_avformat_dec.hpp>
#include <sps_avformat_ts.hpp>

#include <sps_io_bytes.hpp>

namespace sps {

class TsDemuxer : public IAVDemuxer, public IPesHandler {
 public:
    explicit TsDemuxer(PIReader rd);

 public:
    error_t read_header(PSpsAVPacket& buffer)  override;
    error_t read_packet(PSpsAVPacket& buffer)   override;
    error_t read_tail(PSpsAVPacket& buffer)    override;
    error_t probe(PSpsAVPacket& buffer)        override;

 public:
    error_t on_pes_complete(TsPesContext* pes) override;
    error_t on_h264(TsPesContext* pes);
    error_t on_aac(TsPesContext* pes);

 private:
    PAVBuffer buf;
    PSpsBytesReader rd;
    TsContext ts_ctx;

    PSpsAVPacket decoded_pkt;
};

AVInputFormat(Ts, "ts", "ts");

}  // namespace sps

#endif  // SPS_AVFORMAT_TSDEC_HPP

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

#ifndef SPS_AVFORMAT_RTMP_HPP
#define SPS_AVFORMAT_RTMP_HPP

#include <sps_avformat_dec.hpp>
#include <sps_avformat_flv.hpp>

#include <sps_io.hpp>
#include <sps_io_bytes.hpp>

#include <sps_url_rtmp.hpp>

namespace sps {

class RtmpDemuxer : public IAVDemuxer {
 public:
    explicit RtmpDemuxer(PIReader rd);

 public:
    error_t read_header(PSpsAVPacket& buffer)  override;
    error_t read_packet(PSpsAVPacket& buffer)   override;
    error_t read_tail(PSpsAVPacket& buffer)    override;
    error_t probe(PSpsAVPacket& buffer)        override;

 private:
    PRtmpUrlProtocol io;
};

AVInputFormat(Rtmp, "rtmp", "-")

}  // namespace sps

#endif  // SPS_AVFORMAT_RTMP_HPP

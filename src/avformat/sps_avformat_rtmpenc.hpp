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

#ifndef SPS_AVFORMAT_RTMPENC_HPP
#define SPS_AVFORMAT_RTMPENC_HPP

#include <sps_avformat_enc.hpp>
#include <sps_io_url_rtmp.hpp>

namespace sps {

class RtmpAVMuxer : public IAVMuxer {
 public:
    explicit RtmpAVMuxer(PIWriter writer);

 public:
    error_t write_header(PAVPacket& buffer) override;
    error_t write_packet(PAVPacket& buffer) override;
    error_t write_tail(PAVPacket& buffer)    override;

 private:
    PRtmpUrlProtocol  writer;

 public:
    bool      filter_video    = false;
    bool      filter_metadata = false;
    bool      filter_audio    = false;
    int       num             = 0;
};

AVOutputFormat(Rtmp, "rtmp", "-");

}  // namespace sps

#endif  // SPS_AVFORMAT_RTMPENC_HPP

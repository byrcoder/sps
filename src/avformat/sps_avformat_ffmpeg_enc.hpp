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

#ifndef SPS_AVFORMAT_FFMPEG_ENC_HPP
#define SPS_AVFORMAT_FFMPEG_ENC_HPP

#include <sps_auto_header.hpp>
#include <sps_avformat_enc.hpp>
#include <sps_avformat_ffmpeg.hpp>

#ifdef FFMPEG_ENABLED

namespace sps {

class FFmpegAVMuxer : public IAVMuxer {
 public:
    static int write_data(void* opaque, uint8_t* buf, int buf_size);

 public:
    explicit FFmpegAVMuxer(PIWriter writer, PRequestUrl url);

 public:
    error_t write_header(PAVPacket& buffer) override;
    error_t write_packet(PAVPacket& buffer) override;
    error_t write_tail(PAVPacket& buffer)    override;

 public:
    error_t init();

 private:
    error_t init_ffmpeg_ctx();
    void    free_ffmpeg_ctx();

 private:
    PIWriter writer;

    //ffmpeg
    AVFormatContext *ctx;
    AVIOContext* pb;
    uint8_t* avio_ctx_buffer;

    // ffmpeg timestamp
    int64_t cur_timestamp;
};

class FFmpegAVOutputFormat : public IAVOutputFormat {
 public:
    FFmpegAVOutputFormat();
    bool match(const char* ext) const override;

 public:
    PIAVMuxer _create(PIWriter pw) const override;
    PIAVMuxer create2(PIWriter pw, PRequestUrl& url) override;
};

}  // namespace sps

#endif

#endif  // SPS_AVFORMAT_FFMPEG_ENC_HPP

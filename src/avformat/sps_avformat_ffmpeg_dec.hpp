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

#ifndef SPS_AVFORMAT_FFMPEG_DEC_HPP
#define SPS_AVFORMAT_FFMPEG_DEC_HPP

#include <sps_auto_header.hpp>
#include <sps_avformat_dec.hpp>
#include <sps_avformat_ffmpeg.hpp>
#include <sps_avformat_packet.hpp>

#include <sps_io.hpp>
#include <sps_io_bytes.hpp>

#ifdef FFMPEG_ENABLED

namespace sps {

class FFmpegAVDemuxer : public IAVDemuxer {
 public:
    static int nested_io_open(AVFormatContext *s, AVIOContext **pb, const char *url,
                              int flags, AVDictionary **opts);
    static int read_data(void* opaque, uint8_t* buf, int buf_size);

 public:
    FFmpegAVDemuxer(PIReader p);
    ~FFmpegAVDemuxer();

 public:
    error_t read_header(PAVPacket& buffer) override;
    error_t read_packet(PAVPacket& buffer) override;
    error_t read_tail(PAVPacket& buffer) override;
    error_t probe(PAVPacket& buffer) override;

 public:
    error_t init();

 public:
    AVFormatContext* get_ctx();

 private:
    error_t init_ffmpeg_ctx();
    void    free_ffmpeg_ctx();

 private:
    PIReader rd;

    AVFormatContext *ctx;
    AVIOContext* pb;
    uint8_t* avio_ctx_buffer;

    // ffmpeg timestamp
    int64_t cur_timestamp;
};

class FFmpegAVInputFormat : public IAVInputFormat {
 public:
    FFmpegAVInputFormat();

 public:
    bool match(const char* ext) const override;
    PIAVDemuxer _create(PIReader p) override;
};

}  // namespace sps

#endif

#endif  // SPS_AVFORMAT_FFMPEG_DEC_HPP

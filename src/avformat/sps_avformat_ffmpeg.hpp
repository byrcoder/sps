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

#ifndef SPS_AVFORMAT_FFMPEG_HPP
#define SPS_AVFORMAT_FFMPEG_HPP

#define FFMPEG_MAX_SIZE 2 * 1024 * 1024

#include <sps_auto_header.hpp>
#include <sps_avformat_packet.hpp>

#ifdef FFMPEG_ENABLED

extern "C" {

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavformat/avio_internal.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>

}

namespace sps {


const AVRational ffmpeg_1000_time_base = {
    .num = 1,
    .den = 1000
};

class FFmpegPacket : public AVPacket {
 public:
    FFmpegPacket();
    ~FFmpegPacket();

 public:
    ::AVPacket* packet();
    void set_time_base(AVRational time_base);
    const AVRational* get_time_base();

 private:
    ::AVPacket* pkt;
    AVRational time_base;
};

}  // namespace sps

#endif

#endif  // SPS_AVFORMAT_FFMPEG_HPP

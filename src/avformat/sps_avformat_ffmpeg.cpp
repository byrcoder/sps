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

//
// Created by byrcoder on 2022/5/8.
//

#include <sps_avformat_ffmpeg.hpp>

#ifdef FFMPEG_ENABLED

namespace sps {

FFmpegPacket::FFmpegPacket() : AVPacket(0, 0, 0) {
    this->pkt = av_packet_alloc();
    time_base = ffmpeg_1000_time_base;
}

FFmpegPacket::~FFmpegPacket() {
    if (pkt) av_packet_free(&pkt);
}

::AVPacket* FFmpegPacket::packet() {
    return pkt;
}

void FFmpegPacket::set_time_base(AVRational time_base) {
    this->time_base = time_base;
}

const AVRational* FFmpegPacket::get_time_base() {
    return &time_base;
}

}

#endif

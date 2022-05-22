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
#include <sps_log.hpp>

#ifdef FFMPEG_ENABLED

namespace sps {

AVStreamType from_ffmpeg(enum AVMediaType t) {
    switch (t) {
        case AVMEDIA_TYPE_VIDEO:
            return AV_STREAM_TYPE_VIDEO;

        case AVMEDIA_TYPE_AUDIO:
            return AV_STREAM_TYPE_AUDIO;

        default:
            return AV_STREAM_TYPE_NB;
    }
}

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

bool FFmpegPacket::is_video() const {
    return AVPacket::is_video();
}

bool FFmpegPacket::is_audio() const {
    return false;
}

bool FFmpegPacket::is_script() const {
    return false;
}

bool FFmpegPacket::is_keyframe() const {
    return is_video() && (pkt->flags & AV_PKT_FLAG_KEY) != 0;
}

bool FFmpegPacket::is_video_sequence_header() const {
    return false;
}

bool FFmpegPacket::is_audio_sequence_header() {
    return false;
}

bool FFmpegPacket::is_pat() {
    return false;
}

bool FFmpegPacket::is_pmt() {
    return false;
}

bool FFmpegPacket::is_payload_unit_start_indicator() {
    return false;
}

FFmpegAVContext::FFmpegAVContext(AVFormatContext *ctx) {
    this->ctx = ctx;
}

FFmpegAVContext::~FFmpegAVContext() {
    for (auto c : codecs) {
        avcodec_free_context(&c);
    }
}

// https://ffmpeg.org/doxygen/3.4/transcoding_8c-example.html
error_t FFmpegAVContext::init_input() {
    error_t ret = SUCCESS;

    for (int i = 0; i < ctx->nb_streams; ++i) {
        AVStream *stream = ctx->streams[i];
        AVCodec *dec     = nullptr;
        AVCodecContext *codec_ctx = nullptr;
        // subtitle
        if (stream->codecpar->codec_type == AVMEDIA_TYPE_SUBTITLE ||
            stream->codecpar->codec_type == AVMEDIA_TYPE_DATA) {

        } else {
            AVCodec *dec = avcodec_find_decoder(stream->codecpar->codec_id);
            if (!dec) {
                sp_warn("Failed to find decoder for stream #%u(%d-%d)\n", i,
                        stream->codecpar->codec_type, stream->codecpar->codec_id);
                return AVERROR(ENOMEM);
            }
        }

        codec_ctx = avcodec_alloc_context3(dec);
        if (!codec_ctx) {
            sp_error("Failed to allocate the decoder context for stream #%u\n", i);
            return AVERROR(ENOMEM);
        }
        ret = avcodec_parameters_to_context(codec_ctx, stream->codecpar);
        if (ret < 0) {
            sp_error("Failed to copy decoder parameters to input decoder context "
                                       "for stream #%u\n", i);
            return ret;
        }

        sp_trace("Success to find decoder for stream #%u(%d-%d)\n", i,
                stream->codecpar->codec_type, stream->codecpar->codec_id);
        codecs.push_back(codec_ctx);
    }
    return SUCCESS;
}

}

#endif

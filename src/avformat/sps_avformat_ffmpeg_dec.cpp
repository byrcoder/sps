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
// Created by byrcoder on 2022/5/7.
//

#include <sps_avformat_ffmpeg_dec.hpp>
#include <memory>
#include <utility>

#include <sps_avformat_ffmpeg.hpp>

#include <sps_io_bytes.hpp>
#include <sps_log.hpp>

#include <sps_io_url_file.hpp>

#ifdef FFMPEG_ENABLED

namespace sps {

int FFmpegAVDemuxer::nested_io_open(AVFormatContext *s, AVIOContext **pb, const char *url,
                                    int flags, AVDictionary **opts) {
    sp_error("net io has opened");
    return ERROR_FFMPEG_OPEN;
}

int FFmpegAVDemuxer::read_data(void* opaque, uint8_t* buf, int buf_size) {
    auto    ffmpeg = (FFmpegAVDemuxer*) opaque;
    size_t  nr     = 0;
    error_t ret    = ffmpeg->rd->read(buf, buf_size, nr);
    if (ret != SUCCESS) {
        sp_error("fail read ret %d", ret);
        nr = ret > 0 ? -ret : ret;
    }

#ifdef FFMPEG_DECODE_DEBUG
    if (ffmpeg->debug_output && nr > 0) {
        ffmpeg->debug_output->write(buf, nr);
    }
#endif

    sp_debug("demuxer read %lu", nr);
    return (int) nr;
}

FFmpegAVDemuxer::FFmpegAVDemuxer(PIReader p) {
    rd            = std::move(p);
    cur_timestamp = -1;
    init_ffmpeg_ctx();

#ifdef FFMPEG_DECODE_DEBUG
    auto tmp = std::make_shared<FileURLProtocol>(true, false);
    if (tmp->open("decode.flv") != SUCCESS) {
        sp_error("fail open decode.flv");
    } else {
        sp_error("success open decode.flv");
        debug_output = tmp;
    }
#endif
}

FFmpegAVDemuxer::~FFmpegAVDemuxer() {
    free_ffmpeg_ctx();
}

error_t FFmpegAVDemuxer::read_header(PAVPacket& buffer) {
    return SUCCESS;
}

error_t FFmpegAVDemuxer::read_packet(PAVPacket& buffer) {
    auto pkt = std::make_shared<FFmpegPacket>();
    auto ff_pkt = pkt->packet();
    int ret = av_read_frame(ctx, ff_pkt);

    if (ret < 0) {
        sp_error("fail read frame ret %d", ret);
        return ERROR_FFMPEG_READ;
    }

    pkt->set_time_base(ctx->streams[ff_pkt->stream_index]->time_base);
    pkt->stream_type = from_ffmpeg(ctx->streams[ff_pkt->stream_index]->codecpar->codec_type);

    pkt->pts = av_rescale_q_rnd(ff_pkt->pts, ctx->streams[ff_pkt->stream_index]->time_base,
                                       *pkt->get_time_base(), (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
    pkt->dts = av_rescale_q_rnd(ff_pkt->dts, ctx->streams[ff_pkt->stream_index]->time_base,
                                       *pkt->get_time_base(), (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));

    cur_timestamp = av_rescale(pkt->pts, (int64_t) ctx->streams[0]->time_base.num * 90000, ctx->streams[0]->time_base.den);

    buffer = pkt;

    sp_debug("pkt stream_index %d", ff_pkt->stream_index);
    return SUCCESS;
}

error_t FFmpegAVDemuxer::read_tail(PAVPacket& buffer) {
    return SUCCESS;
}

error_t FFmpegAVDemuxer::probe(PAVPacket& buffer) {
    return SUCCESS;
}

IAVContext* FFmpegAVDemuxer::get_av_ctx() {
    return ffmpeg_av_ctx.get();
}

error_t FFmpegAVDemuxer::init() {
    ff_const59 AVInputFormat *in_fmt = nullptr;
    AVDictionary  *in_fmt_opts       = nullptr;
    int            ret               = 0;
    uint8_t        *avio_ctx_buffer  = (uint8_t*) av_malloc(FFMPEG_MAX_SIZE);
    ctx                              = avformat_alloc_context();

    if (!avio_ctx_buffer) {
        sp_error("fail av_malloc %d", FFMPEG_MAX_SIZE);
        return ERROR_FFMPEG_OPEN;
    }

    pb             = avio_alloc_context(avio_ctx_buffer, FFMPEG_MAX_SIZE, 0,
                                          this, read_data, NULL, NULL);
    ctx->flags     = AVFMT_FLAG_CUSTOM_IO;
    ctx->probesize = 1024 * 1024 * 1;
    ctx->pb        = pb;
    ctx->io_open   = nested_io_open;
    ctx->format_probesize     = 1024 * 1024;
    ctx->max_analyze_duration = 4 * AV_TIME_BASE;

    ret = avformat_open_input(&ctx, "", in_fmt, &in_fmt_opts);
    av_dict_free(&in_fmt_opts);

    if (ret < 0) {
        sp_error("fail avformat_open_input ret %d", ret);
        return ERROR_FFMPEG_OPEN;
    }

    ret = avformat_find_stream_info(ctx, nullptr);
    if (ret < 0) {
        return ERROR_FFMPEG_OPEN;
    }

    // https://ffmpeg.org/doxygen/3.4/transcoding_8c-example.html
    ffmpeg_av_ctx = std::make_shared<FFmpegAVContext>(ctx);
    if ((ret = ffmpeg_av_ctx->init_input()) != SUCCESS) {
        return ret;
    }

    return SUCCESS;
}

AVFormatContext* FFmpegAVDemuxer::get_ctx() {
    return ctx;
}

error_t FFmpegAVDemuxer::init_ffmpeg_ctx() {
    pb  = nullptr;
    ctx = nullptr;
    return SUCCESS;
}

void FFmpegAVDemuxer::free_ffmpeg_ctx() {
    // https://ffmpeg.org/pipermail/libav-user/2012-December/003257.html
    // avio_ctx_buffer manage by pb
    if (pb->buffer) av_freep(&pb->buffer);
    if (pb) avio_context_free(&pb);
    if (ctx) avformat_free_context(ctx);
}

FFmpegAVInputFormat::FFmpegAVInputFormat() : IAVInputFormat("ffmpeg", ".*") {
}

bool FFmpegAVInputFormat::match(const char* ext) const {
    return true;
}

PIAVDemuxer FFmpegAVInputFormat::_create(PIReader p) {
    auto demuxer = std::make_shared<FFmpegAVDemuxer>(p);
    // ffmpeg init here avformat_input

    if (demuxer->init() != SUCCESS) {
        return nullptr;
    }

    return demuxer;
}

}

#endif

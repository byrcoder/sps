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

#include <sps_avformat_ffmpeg_enc.hpp>
#include <sps_avformat_ffmpeg.hpp>

#include <sps_log.hpp>
#include <sps_url_file.hpp>

#ifdef FFMPEG_ENABLED

namespace sps {

int FFmpegAVMuxer::write_data(void* opaque, uint8_t* buf, int buf_size) {
    FFmpegAVMuxer* ffmpeg = (FFmpegAVMuxer*) opaque;
    int nr = 0;
    error_t ret = ffmpeg->writer->write((void*) buf, buf_size);
    if (ret != SUCCESS) {
        nr = ret > 0 ? -ret : ret;
    } else {
        nr = buf_size;
    }

#ifdef FFMPEG_ENCODE_DEBUG
    if (ffmpeg->debug_output && nr > 0) {
        ffmpeg->debug_output->write(buf, nr);
    }
#endif

    sp_debug("write_data nr %d", nr);
    return nr;
}

FFmpegAVMuxer::FFmpegAVMuxer(PIWriter writer, PRequestUrl url) : writer(std::move(writer)) {
    this->url = std::move(url);
    init_ffmpeg_ctx();

#ifdef FFMPEG_ENCODE_DEBUG
    auto tmp = std::make_shared<FileURLProtocol>(true, false);
    if (tmp->open("tmp.flv") != SUCCESS) {
        sp_error("fail open tmp.flv");
    } else {
        sp_error("success open tmp.flv");
        debug_output = tmp;
    }
#endif
}

FFmpegAVMuxer::~FFmpegAVMuxer() {
    free_ffmpeg_ctx();
}

error_t FFmpegAVMuxer::write_header(PAVPacket &buffer) {
    int ret = avformat_write_header(ctx, nullptr);
    if (ret < 0) {
        sp_error("fail write header %d, url %s", ret, url->get_url());
        return ERROR_FFMPEG_WRITE;
    }
    return SUCCESS;
}

error_t FFmpegAVMuxer::write_packet(PAVPacket& buffer) {
    auto pkt = dynamic_cast<FFmpegPacket*>(buffer.get());

    if (pkt->packet()->stream_index >= stream_mapping.size() ||
        stream_mapping[pkt->packet()->stream_index] < 0) {
        sp_warn("ignore packet index %d", pkt->packet()->stream_index);
        return SUCCESS;
    }

    ::AVPacket out_pkt;

    int ret = av_packet_ref(&out_pkt, pkt->packet());

    /* copy packet */
    av_packet_rescale_ts(&out_pkt, *pkt->get_time_base(), ctx->streams[pkt->packet()->stream_index]->time_base);
    out_pkt.pos = -1;

    ret = av_write_frame(ctx, &out_pkt);
    av_packet_unref(&out_pkt);
    if (ret < 0) {
        sp_error("fail write pkt %d", ret);
        return ERROR_FFMPEG_WRITE;
    }

    return SUCCESS;
}

error_t FFmpegAVMuxer::write_tail(PAVPacket &buffer) {
    return SUCCESS;
}

error_t FFmpegAVMuxer::set_av_ctx(IAVContext* c) {
    auto ffmpeg_ctx = dynamic_cast<FFmpegAVContext*>(c);
    int  ret        = SUCCESS;

    if (!ffmpeg_ctx) {
        sp_warn("set av ctx empty!");
        return SUCCESS;
    }

    for (int i = 0; i < ffmpeg_ctx->ctx->nb_streams; i++) {
        if ((ret = on_av_stream(ffmpeg_ctx->ctx->streams[i])) != SUCCESS) {
            sp_error( "fail init ctx stream %d < %d", i, ffmpeg_ctx->ctx->nb_streams);
            break;
        }
    }

    if (ret == SUCCESS) {
        av_dump_format(get_ctx(), 0, nullptr, 1);
    }

    return ret;
}

error_t FFmpegAVMuxer::init() {
    // TODO: impl
    ff_const59 AVOutputFormat* out_format = av_guess_format(nullptr, url->get_url(), nullptr);
    int ret = avformat_alloc_output_context2(&ctx, out_format, nullptr, nullptr);

    if (ret < 0) {
        sp_error("ffmpeg output %s ret %d", url->url.c_str(), ret);
        return ERROR_FFMPEG_OPEN;
    }

    avio_ctx_buffer  = (uint8_t*) av_malloc(FFMPEG_MAX_SIZE);
    pb               = avio_alloc_context(avio_ctx_buffer, FFMPEG_MAX_SIZE, 1,
                                          this, nullptr, write_data, nullptr);
    ctx->pb          = pb;
    ctx->oformat     = out_format;

    return SUCCESS;
}

AVFormatContext *FFmpegAVMuxer::get_ctx() {
    return ctx;
}

// https://github.com/FFmpeg/FFmpeg/blob/master/doc/examples/remuxing.c
error_t FFmpegAVMuxer::on_av_stream(AVStream* stream) {
    AVCodecParameters *in_codecpar = stream->codecpar;

    if (in_codecpar->codec_type != AVMEDIA_TYPE_AUDIO &&
        in_codecpar->codec_type != AVMEDIA_TYPE_VIDEO &&
        in_codecpar->codec_type != AVMEDIA_TYPE_SUBTITLE &&
        in_codecpar->codec_type != AVMEDIA_TYPE_DATA) {
        sp_warn( "ignore code type %d\n", in_codecpar->codec_type);
        stream_mapping.push_back(-1);
        return SUCCESS;
    }

    AVStream *out_stream = avformat_new_stream(ctx, nullptr);

    if (!out_stream) {
        sp_error( "Failed allocating output stream\n");
        return ERROR_FFMPEG_WRITE;
    }

    int ret = avcodec_parameters_copy(out_stream->codecpar, in_codecpar);
    if (ret < 0) {
        sp_error( "Failed to copy context from input to output stream codec context");
        return ERROR_FFMPEG_WRITE;
    }

    out_stream->codecpar->codec_tag = 0;
    stream_mapping.push_back(stream_mapping.size());
    sp_trace("success copy context input %d", stream->codecpar->codec_id);

    return SUCCESS;
}

#if 0

error_t FFmpegAVMuxer::on_av_stream(AVStream* new_stream) {
    auto codec               = avcodec_find_encoder(new_stream->codecpar->codec_id);

    if (!codec) {
        sp_error( "Failed to copy context from input to output stream codec context");
        return ERROR_FFMPEG_WRITE;
    }

    AVStream *out_stream     = avformat_new_stream(ctx, codec);

    if (!out_stream) {
        sp_error( "Failed allocating output stream\n");
        return ERROR_FFMPEG_WRITE;
    }

    avcodec_parameters_copy(out_stream->codecpar, new_stream->codecpar);
    out_stream->sample_aspect_ratio = new_stream->sample_aspect_ratio;
    out_stream->time_base           = ffmpeg_1000_time_base;
    out_stream->avg_frame_rate      = new_stream->avg_frame_rate;

    return SUCCESS;
}

#endif

error_t FFmpegAVMuxer::init_ffmpeg_ctx() {
    pb = nullptr;
    avio_ctx_buffer = nullptr;
    ctx = nullptr;
    return SUCCESS;
}

void FFmpegAVMuxer::free_ffmpeg_ctx() {
    if (pb) avio_context_free(&pb);

    // if (avio_ctx_buffer) av_free(avio_ctx_buffer);

    if (ctx) avformat_free_context(ctx);
}

FFmpegAVOutputFormat::FFmpegAVOutputFormat() : IAVOutputFormat("ffmpeg", "*") {
}

bool FFmpegAVOutputFormat::match(const char *ext) const {
    return true;
}

PIAVMuxer FFmpegAVOutputFormat::_create(PIWriter pw) const {
    // must contain url to know the output format
    return nullptr;
}

PIAVMuxer FFmpegAVOutputFormat::create2(PIWriter pw, PRequestUrl &url) {
    auto ffmpeg = std::make_shared<FFmpegAVMuxer>(std::move(pw), url);
    int ret     = ffmpeg->init();

    if (ret != SUCCESS) {
        sp_error("fail init ffmpeg output ret %d", ret);
        return nullptr;
    }

    return ffmpeg;
}

}

#endif


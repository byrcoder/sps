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
    int ret = av_write_frame(ctx, pkt->packet());
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

error_t FFmpegAVMuxer::on_av_stream(AVStream* stream) {
    AVCodec *dec     = nullptr;
    AVCodecContext *codec_ctx = nullptr;
    // subtitle
    if (stream->codecpar->codec_type == AVMEDIA_TYPE_SUBTITLE ||
        stream->codecpar->codec_type == AVMEDIA_TYPE_DATA) {

    } else {
        AVCodec *dec = avcodec_find_encoder(stream->codecpar->codec_id);
        if (!dec) {
            sp_warn("Failed to find encoder for stream #%u(%d-%d)\n", stream->index,
                    stream->codecpar->codec_type, stream->codecpar->codec_id);
            return AVERROR(ENOMEM);
        }
    }

    AVCodecContext *code_ctx = avcodec_alloc_context3(dec);
    int ret = avcodec_parameters_to_context(code_ctx, stream->codecpar);

    if (!code_ctx || ret < 0) {
        avcodec_free_context(&code_ctx);
        sp_error("fail to copy context input %d", stream->codecpar->codec_id);
        return ERROR_FFMPEG_WRITE;
    }

    AVStream *out_stream = avformat_new_stream(ctx, dec);

    if (!out_stream) {
        avcodec_free_context(&code_ctx);
        sp_error( "Failed allocating output stream\n");
        return ERROR_FFMPEG_WRITE;
    }

    code_ctx->codec_tag = 0;
    if (ctx->oformat->flags & AVFMT_GLOBALHEADER) {
        code_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    ret = avcodec_parameters_from_context(out_stream->codecpar, code_ctx);
    if (ret < 0) {
        avcodec_free_context(&code_ctx);
        sp_error( "Failed to copy context from input to output stream codec context");
        return ERROR_FFMPEG_WRITE;
    }

    out_stream->sample_aspect_ratio = stream->sample_aspect_ratio;
    out_stream->time_base           = stream->time_base;
    out_stream->avg_frame_rate      = stream->avg_frame_rate;

    avcodec_free_context(&code_ctx);

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

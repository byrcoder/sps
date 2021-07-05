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
// Created by byrcoder on 2021/6/15.
//

#include <sps_stream_phase_handler.hpp>

#include <sps_avformat_dec.hpp>
#include <sps_avformat_enc.hpp>

namespace sps {

StreamPhaseHandler::StreamPhaseHandler() : IPhaseHandler("stream-handler"){
}

error_t StreamPhaseHandler::handler(ConnContext &ctx) {
    auto    stream_ctx = std::static_pointer_cast<StreamConfCtx>(ctx.host->stream_module->conf);
    error_t ret        = SUCCESS;

    if (!stream_ctx || !stream_ctx->edge) {
        sp_error("fatal not support stream publish source!");
        return ERROR_AVFORMAT_SOURCE_NOT_SUPPORT;
    }

    auto& demuxers = SingleInstance<AVDemuxerFactory>::get_instance();
    auto& encoders = SingleInstance<AVEncoderFactory>::get_instance();

    auto enc = encoders.create(ctx.socket, ctx.req);

    if (!enc) {
        sp_error("failed found encoder for ext %s", ctx.req->ext.c_str());
        return ERROR_AVFORMAT_ENCODER_NOT_EXISTS;
    }

    auto upstream_req  = stream_ctx->pass_proxy + ctx.req->path;
    if (!ctx.req->params.empty()) {
        upstream_req += "?" + ctx.req->params;
    }

    sp_info("upstream url %s.", upstream_req.c_str());

    PIURLProtocol url_protocol = SingleInstance<UrlProtocol>::get_instance().create(upstream_req);
    if (!url_protocol) {
        sp_error("failed found url protocol for %s.", upstream_req.c_str());
        return ERROR_URL_PROTOCOL_NOT_EXISTS;
    }

    auto dec = demuxers.create(url_protocol, upstream_req);

    if (!dec) {
        sp_error("failed found encoder for ext %s", ctx.req->ext.c_str());
        return ERROR_AVFORMAT_DEMUX_NOT_EXISTS;
    }

    PSpsAVPacket buffer;
    ret = dec->read_header(buffer);

    if (ret != SUCCESS) {
        sp_error("failed dec read header url protocol for ret:%d", ret);
        return ret;
    }

    ret = enc->write_header(buffer);

    if (ret != SUCCESS) {
        sp_error("failed encoder write header url protocol for ret:%d", ret);
        return ret;
    }

    do {
        ret = dec->read_packet(buffer);

        if (ret != SUCCESS) {
            sp_error("failed dec read message url protocol for ret:%d", ret);
            break;
        }

        ret = enc->write_message(buffer);

        if (ret != SUCCESS) {
            sp_error("failed encoder write message url protocol for ret:%d", ret);
            break;
        }

    } while(ret == SUCCESS);

    return ret;
}

}
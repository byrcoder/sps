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
// Created by byrcoder on 2021/7/16.
//

#include <sps_http_stream_phase_handler.hpp>

#include <sps_avformat_cache.hpp>
#include <sps_avformat_dec.hpp>
#include <sps_avformat_enc.hpp>

#include <sps_http_socket.hpp>

#include <sps_stream_cache.hpp>

namespace sps {

std::string HttpStreamPhaseHandler::get_cache_key(PRequestUrl& req) {
    std::string path   = req->get_path();
    std::string url    = req->host + (req->ext.empty() ? path
            : path.substr(0, path.size()-req->ext.size()-1));
    return url;
}

HttpStreamPhaseHandler::HttpStreamPhaseHandler()
    : IPhaseHandler("http-stream-handler") {
}

error_t HttpStreamPhaseHandler::handler(ConnContext& ctx) {
    error_t ret  = SUCCESS;

    if (!ctx.host->is_streaming()) {
        sp_error("fatal not support streaming!");
        return ERROR_AVFORMAT_SOURCE_NOT_SUPPORT;
    }

    if (ctx.req->method == "POST") {
        return handler_publish(ctx);
    }

    if (!ctx.host->publish()) {
        auto rsp = std::make_shared<HttpResponseSocket>(ctx.socket,
            ctx.ip, ctx.port);

        rsp->init(500, nullptr, 0, false);
        ret = rsp->write_header();
        sp_error("not support proxy for streaming! http rsp %d, ret %d",
                 500, ret);
        return ERROR_AVFORMAT_SOURCE_NOT_SUPPORT;
    }

    return handler_play(ctx);
}

error_t HttpStreamPhaseHandler::handler_play(ConnContext& ctx) {
    error_t ret  = SUCCESS;
    auto rsp     = std::make_shared<HttpResponseSocket>(ctx.socket,
        ctx.ip, ctx.port);

    std::string url   = get_cache_key(ctx.req);
    auto cache        =  StreamCache::get_streamcache(url);

    if (!cache) {
        rsp->init(404, nullptr, 0, false);
        ret = rsp->write_header();
        sp_error("not publishing for streaming! url %s http rsp %d, ret %d, method %s",
                 url.c_str(), 404, ret, ctx.req->get_method());
        return ret;
    }

    rsp->init(200, nullptr, -1, false);

    auto& encoder = SingleInstance<AVEncoderFactory>::get_instance();
    auto enc      = encoder.create(rsp, ctx.req);
    auto cc       = StreamCache::get_client_streamcache(url, 10 * 1000 * 1000);

    if (!enc) {
        sp_error("failed found encoder for ext %s", ctx.req->ext.c_str());
        return ERROR_AVFORMAT_ENCODER_NOT_EXISTS;
    }

    // support ffmpeg ctx
    enc->set_av_ctx((IAVContext*) cache->get_ctx());

    PAVPacket buffer;
    ret = enc->write_header(buffer);

    if (ret != SUCCESS) {
        sp_error("failed encoder write header url protocol for ret:%d", ret);
        return ret;
    }

    do {
        std::list<PAVPacket> vpb;
        int n = cc->dump(vpb, false);

        if (n == 0) {
            ret = ERROR_SOCKET_TIMEOUT;
            sp_error("fail timeout playing recv ret %d", ret);
            break;
        }

        for (auto& p : vpb) {
            if ((ret = enc->write_packet(p)) != SUCCESS) {
                sp_error("fail encoder write message url protocol for ret:%d",
                         ret);
                break;
            }

            sp_debug("write packet!");
        }
    } while (ret == SUCCESS);

    return ret;
}

error_t HttpStreamPhaseHandler::handler_publish(ConnContext& ctx) {
    error_t ret  = SUCCESS;
    auto rsp     = std::make_shared<HttpResponseSocket>(ctx.socket, ctx.ip, ctx.port);

    if (!ctx.host->publish()) {
        rsp->init(500, nullptr, 0, false);
        ret = rsp->write_header();
        sp_error("not support http publish for streaming! http rsp %d, ret %d",
                 500, ret);
        return ERROR_AVFORMAT_SOURCE_NOT_SUPPORT;
    }

    if (!ctx.host->support_publish(ctx.req->ext)) {
        rsp->init(404, nullptr, 0, false);
        ret = rsp->write_header();
        sp_error("not support http format(publishing) for streaming %s! "
                 "http rsp %d, ret %d, source_format %s",
                 ctx.req->get_ext(), 500, ret, ctx.host->edge_format().c_str());
        return ERROR_AVFORMAT_SOURCE_NOT_SUPPORT;
    }

    auto& decoder = SingleInstance<AVDemuxerFactory>::get_instance();
    auto  dec     = decoder.create(ctx.socket, ctx.req);

    if (!dec) {
        sp_error("failed found decoder for ext %s", ctx.req->ext.c_str());
        return ERROR_AVFORMAT_DEMUX_NOT_EXISTS;
    }

    sp_info("success found decoder for %s, %s",
             ctx.req->get_ext(), dec->fmt->name);

    std::string url    =  get_cache_key(ctx.req);
    auto cache         =  StreamCache::get_streamcache(url);

    if (cache) {
        sp_error("cannot publish twice");
        return ERROR_HTTP_HAS_SOURCE;
    }

    cache = StreamCache::create_av_streamcache(url);
    sp_trace("Publish url %s", url.c_str());

    PAVPacket packet;

    // support ffmpeg ctx
    cache->set_ctx(dec->get_av_ctx());

    if ((ret = dec->read_header(packet)) != SUCCESS) {
        sp_error("fail read header recv ret %d", ret);
        goto final;
    }

    do {
        if ((ret = dec->read_packet(packet)) != SUCCESS) {
            sp_error("fail publishing recv ret %d", ret);
            break;
        }

        // packet->debug();
        cache->put(packet);
    } while (true);

final:
    StreamCache::release_av_streamcache(url);
    return ret;  // ignore
}

}  // namespace sps

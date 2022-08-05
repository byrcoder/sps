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
// Created by byrcoder on 2022/8/5.
//

#include <sps_stream_handler.hpp>

#include <sps_avformat_dec.hpp>

#include <sps_stream.hpp>
#include <sps_stream_cache.hpp>
#include <sps_stream_edge.hpp>

namespace sps {

std::string StreamHandler::get_cache_key(PRequestUrl& req) {
    std::string path = req->get_path();
    std::string url  = req->host + (req->ext == "-" || req->ext.empty() ? path :
                        path.substr(0, path.size()- req->ext.size() -1));
    return url;
}

StreamHandler::StreamHandler(std::shared_ptr<Socket> io, bool pub) :
    IPhaseHandler("stream-handler"), io(std::move(io)) {
    this->pub = pub;
}

error_t StreamHandler::handler(ConnContext &ctx) {
    return pub ? publish(ctx) : play(ctx);
}

error_t StreamHandler::publish(ConnContext &ctx) {
    if (!ctx.host->publish() || !ctx.host->support_publish(ctx.req->ext)) {
        sp_error("not support publish! %s", ctx.req->get_host());
        return ERROR_STREAM_PUBLISH_NOT_SUPPORT;
    }

    auto& decoder = SingleInstance<AVDemuxerFactory>::get_instance();
    auto  dec     = decoder.create(io, ctx.req);

    if (!dec) {
        sp_error("failed found decoder for ext %s", ctx.req->ext.c_str());
        return ERROR_AVFORMAT_DEMUX_NOT_EXISTS;
    }

    sp_info("success found decoder for %s[%s], %s",
            ctx.req->get_url(), ctx.req->get_ext(), dec->fmt->name);

    std::string url    =  get_cache_key(ctx.req);
    auto cache         =  StreamCache::get_streamcache(url);

    if (cache) {
        sp_error("cannot publish twice");
        return ERROR_HTTP_HAS_SOURCE;
    }

    cache = StreamCache::create_av_streamcache(url);
    sp_trace("Publish url %s", url.c_str());

    StreamDecoder stream_decode(dec, cache);
    error_t ret = stream_decode.decode();
    StreamCache::release_av_streamcache(url);
    return ret;  // ignore
}

error_t StreamHandler::play(ConnContext &ctx) {
    error_t     ret     = SUCCESS;
    std::string url     = get_cache_key(ctx.req);
    auto        cache   = StreamCache::get_streamcache(url);

    if (!cache && ctx.host->edge()) {
        ret   = StreamEdgeEnter::start_edge(url, ctx.host, ctx.req);
        cache = StreamCache::get_streamcache(url);
    }

    if (!cache || ret != SUCCESS) {
        sp_error("not publishing or edge for streaming! url %s http rsp %d, ret %d, method %s",
                 url.c_str(), 404, ret, ctx.req->get_method());
        return ERROR_STREAM_SOURCE_NOT_EXITS;
    }

    auto& encoder = SingleInstance<AVEncoderFactory>::get_instance();
    auto enc      = encoder.create(io, ctx.req);

    if (!enc) {
        sp_error("Failed found encoder for ext %s", ctx.req->ext.c_str());
        return ERROR_AVFORMAT_ENCODER_NOT_EXISTS;
    }
    auto cc  = StreamCache::get_client_streamcache(url, 10 * 1000 * 1000);

    // support ffmpeg ctx
    enc->set_av_ctx((IAVContext*) cache->get_ctx());
    StreamEncoder stream_encoder(enc, cc);
    ret = stream_encoder.encode();
    cache->cancel(cc);

    return ret;
}

}

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
// Created by byrcoder on 2021/8/24.
//

#include <sps_srt_stream_handler.hpp>
#include <sps_avformat_tsdec.hpp>

#ifndef SRT_DISABLED

namespace sps {

SrtServerStreamHandler::SrtServerStreamHandler()
        : IPhaseHandler("srt-server") {
}

error_t SrtServerStreamHandler::handler(ConnContext &ctx) {
    auto mode = ctx.req->get_param("m");

    sp_info("sever srt host: %s, mode %s",  ctx.req->get_host(), mode.c_str());

    if (ctx.req->method == "request") {
        return play(ctx);
    } else {
        return publish(ctx);
    }
}

error_t SrtServerStreamHandler::publish(ConnContext &ctx) {

    std::string url    =  ctx.req->host + ctx.req->get_path();
    std::string ts_url = "ts://" + url;
    auto cache         = StreamCache::get_streamcache(url);
    error_t     ret    = SUCCESS;

    if (cache) {
        sp_error("cannot publish twice %s", url.c_str());
        return ERROR_RTMP_HAS_SOURCE;
    }

    cache = StreamCache::create_av_streamcache(url);
    sp_trace("Publish url %s", url.c_str());

    // only for ts
    auto ts_cache = StreamCache::create_raw_streamcache(ts_url);
    auto ts_handler = std::make_shared<TsCache>(ts_cache);
    TsDemuxer demuxer(ctx.socket, ts_handler);

    do {
        PAVPacket packet;
        if ((ret = demuxer.read_packet(packet)) != SUCCESS) {
            sp_error("fail publishing recv ret %d", ret);
            break;
        }
        cache->put(packet);
    } while (true);

final:
    StreamCache::release_av_streamcache(url);
    StreamCache::release_av_streamcache(ts_url);
    return ret;  // ignore
}

error_t SrtServerStreamHandler::play(ConnContext &ctx) {
    std::string url    =  ctx.req->host + ctx.req->get_path();
    std::string ts_url = "ts://" + url;
    auto cache         = StreamCache::get_streamcache(ts_url);
    error_t     ret    = SUCCESS;

    if (!cache) {
        sp_error("cannot not publising %s", url.c_str());
        return ERROR_RTMP_HAS_SOURCE;
    }
    auto cc          =  StreamCache::get_client_streamcache(ts_url, 10 * 1000 * 1000);

    do {
        std::list<PAVPacket> vpb;
        int n = cc->dump(vpb, false);

        if (n == 0) {
            ret = ERROR_SOCKET_TIMEOUT;
            sp_error("fail timeout playing recv ret %d", ret);
            break;
        }

        for (auto& p : vpb) {
            if ((ret = ctx.socket->write(p->buffer(), p->size())) != SUCCESS) {
                sp_error("fail playing send ret %d", ret);
                break;
            }
            // sp_info("write pkt %d", p->size());
        }
    } while (ret == SUCCESS);

final:
    cache->cancel(cc);
    return ret;  // ignore
}

}

#endif

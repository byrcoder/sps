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
// Created by byrcoder on 2021/7/4.
//

#include <sps_rtmp_stream_handler.hpp>

#include <list>
#include <memory>
#include <string>
#include <utility>

#include <sps_avformat_rtmpdec.hpp>
#include <sps_avformat_rtmpenc.hpp>

#include <sps_librtmp_packet.hpp>
#include <sps_rtmp_server.hpp>
#include <sps_rtmp_server_handler.hpp>

namespace sps {

RtmpServerStreamHandler::RtmpServerStreamHandler()
    : IPhaseHandler("rtmp-server") {
}

error_t RtmpServerStreamHandler::handler(ConnContext &ctx) {
    auto     rt  = dynamic_cast<RtmpConnHandler*>(ctx.conn);
    error_t  ret = SUCCESS;
    RtmpServerHandshake shk(rt->hk.get());

    sp_info("sever rtmp host: %s",  ctx.req->get_host());

    if (rt->publishing) {
        return publish(ctx);
    } else if (rt->playing) {
        return play(ctx);
    } else {
        sp_error("fatal unknown rtmp role");  // never happen
        ret = ERROR_RTMP_ROLE;
    }

    return ret;
}

error_t RtmpServerStreamHandler::publish(ConnContext &ctx) {
    // cannot publish when edge
    if (!ctx.host->is_streaming()) {
        sp_error("fatal not support rtmp stream publish source! conf null");
        return ERROR_STREAM_NOT_CONF;
    }

    if (!ctx.host->publish()) {
        sp_error("fatal edge cannot publish format: %s, role: %s",
                 ctx.host->edge_format().c_str(),
                  ctx.host->role().c_str());
        return ERROR_AVFORMAT_SOURCE_NOT_SUPPORT;
    }

    if (!ctx.host->support_publish("rtmp")) {
        sp_error("not support rtmp publish format: %s, role: %s",
                 ctx.host->edge_format().c_str(),
                 ctx.host->role().c_str());
        return ERROR_AVFORMAT_SOURCE_NOT_SUPPORT;
    }

    auto    rt  = dynamic_cast<RtmpConnHandler*>(ctx.conn);
    error_t ret = SUCCESS;
    auto    io  = std::make_shared<RtmpUrlProtocol>(rt->hk);
    RtmpDemuxer demuxer(io);

    std::string url    =  ctx.req->host + ctx.req->get_path();
    auto cache         = StreamCache::get_streamcache(url);

    if (cache) {
        sp_error("cannot publish twice");
        return ERROR_RTMP_HAS_SOURCE;
    }

    cache = StreamCache::create_av_streamcache(url);
    sp_trace("Publish url %s", url.c_str());

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
    return ret;  // ignore
}

error_t RtmpServerStreamHandler::play(ConnContext &ctx) {
    auto    rt  = dynamic_cast<RtmpConnHandler*>(ctx.conn);
    error_t ret = SUCCESS;

    std::string url  =  ctx.req->host + ctx.req->get_path();
    auto cache       =  StreamCache::get_streamcache(url);
    auto cc          =  StreamCache::get_client_streamcache(url, 10 * 1000 * 1000);

    rt->hk->set_recv_timeout(SPS_IO_NO_TIMEOUT);

    if (!ctx.host->is_streaming()) {
        sp_error("rtmp stream has no config!");
        return ERROR_STREAM_NOT_CONF;
    }

    if (!cache) {
        // cannot publish when edge
        if (ctx.host->publish()) {
            sp_error("rtmp stream has no source! url %s", url.c_str());
            return ERROR_RTMP_NO_SOURCE;
        }

        sp_error("rtmp edge not support!");
        return ERROR_RTMP_NOT_IMPL;
    }
    auto    io  = std::make_shared<RtmpUrlProtocol>(rt->hk);
    RtmpAVMuxer muxer(io);

    sp_trace("Playing url %s", url.c_str());

    do {
        std::list<PAVPacket> vpb;
        int n = cc->dump(vpb, false);

        if (n == 0) {
            ret = ERROR_SOCKET_TIMEOUT;
            sp_error("fail timeout playing recv ret %d", ret);
            break;
        }

        for (auto& p : vpb) {
            if ((ret = muxer.write_message(p)) != SUCCESS) {
                sp_error("fail playing send ret %d", ret);
                break;
            }
        }
    } while (ret == SUCCESS);

final:
    cache->cancel(cc);
    return ret;  // ignore
}

}  // namespace sps

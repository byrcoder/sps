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
// Created by byrcoder on 2021/6/22.
//

#include <sps_rtmp_server_handler.hpp>
#include <sps_avformat_rtmpdec.hpp>

#include <sps_avformat_librtmp_packet.hpp>
#include <sps_rtmp_server.hpp>

namespace sps {

RtmpServer404Handler::RtmpServer404Handler()
    : IPhaseHandler("rtmp-404-http_server") {
}

error_t RtmpServer404Handler::handler(IConnection &c) {
    auto& ctx = *dynamic_cast<HostContext*>(c.get_context().get());
    sp_error("Fail found host: %s", ctx.req->get_host());
    return ERROR_UPSTREAM_NOT_FOUND;
}

RtmpPrepareHandler::RtmpPrepareHandler()
    : IPhaseHandler("rtmp-handshake-http_server") {
}

error_t RtmpPrepareHandler::handler(IConnection &c) {
    auto     hook =  std::make_shared<RtmpHook>(c.socket);;
    error_t  ret  = SUCCESS;
    auto     ctx  = std::make_shared<RtmpContext>(nullptr, c.socket);
    RtmpServerHandshake shk(hook.get());

    // handshake
    if ((ret = shk.handshake()) != SUCCESS) {
        sp_error("Fail s-handshake %d", ret);
        return ERROR_RTMP_HANDSHAKE;
    }

    // connect
    RtmpPreRequest pre(hook.get());
    if ((ret = pre.connect()) != SUCCESS) {
        sp_error("Fail s-connect %d", ret);
        return ret;
    }

    ctx->playing    = pre.playing;
    ctx->publishing = pre.publishing;
    ctx->req        = std::make_shared<RequestUrl>();
    auto tc_url     = pre.tc_url + pre.stream_params;
    if ((ret = ctx->req->parse_url(tc_url)) != SUCCESS) {
        sp_error("Fail parse tc_url %s %d", tc_url.c_str(), ret);
        return ret;
    }

    sp_trace("Request client %s:%d, %s", c.socket->get_peer_ip().c_str(),
             c.socket->get_peer_port(), tc_url.c_str());
    c.set_context(ctx);

    return SUCCESS;
}

RtmpServerHandshake::RtmpServerHandshake(RtmpHook* r) {
    this->hook = r;
}

error_t RtmpServerHandshake::handshake() {
    return hook->server_handshake();
}

RtmpPreRequest::RtmpPreRequest(RtmpHook* r) {
    this->hook = r;
}

error_t RtmpPreRequest::connect() {
    error_t   ret  = SUCCESS;

    WrapRtmpPacket   pkt;
    PIRTMPPacket     connect_packet;

    do {
        if ((ret = hook->recv_packet(pkt)) != SUCCESS) {
            sp_error("Fail rcv paket %d", ret);
            return ret;
        }

        ret = RtmpPacketDecoder::decode(pkt, connect_packet);
        if (ret != SUCCESS) {
            sp_error("Fail decoded paket %d", ret);
            return ret;
        }

        auto conn_packet = dynamic_cast<ConnectRtmpPacket *>(connect_packet.get());
        if (!conn_packet) {
            // ret = ERROR_RTMP_AMF_DECODE;
            sp_error("Fatal expect ConnectRtmpPacket");
            continue;;
        }

        tc_url = std::string(conn_packet->tc_url.av_val,
                         conn_packet->tc_url.av_len);
        txn    = conn_packet->transaction_id;
        break;
    } while (true);

    ret = hook->send_ack_window_size();
    if (ret != SUCCESS) {
        return ret;
    }

    ret = hook->send_peer_bandwidth();
    if (ret != SUCCESS) {
        return ret;
    }

    if (((ret = hook->send_set_chunked_size()) != SUCCESS) ||
            ((ret = hook->send_connect_result(txn)) != SUCCESS)) {
        sp_error("Failed send connect result %d", ret);
        return ret;
    }

    while ((ret = hook->recv_packet(pkt)) == SUCCESS) {
        PIRTMPPacket     conn;
        ret = RtmpPacketDecoder::decode(pkt, conn);

        if (ret != SUCCESS) {
            sp_error("Fail decode pkt_type %d %d", pkt.packet.m_packetType, ret);
            return ret;
        }

        // create_stream
        auto create_stream = dynamic_cast<CreateStreamRtmpPacket*>(conn.get());
        if (create_stream) {
            if ((ret = on_recv_create_stream(create_stream)) != SUCCESS) {
                return ret;
            }
            continue;
        }

        // playing
        auto play = dynamic_cast<PlayRtmpPacket*>(conn.get());
        if (play) {
            if ((ret = on_recv_play(play)) != SUCCESS) {
                return ret;
            }

            playing = true;
            break;
        }

        // FCPublish
        auto fcpublish = dynamic_cast<FCPublishRtmpPacket*>(conn.get());
        if (fcpublish) {
            if ((ret = on_recv_fcpublish(fcpublish)) != SUCCESS) {
                return ret;
            }
            continue;
        }

        // releaseStream
        auto release_stream = dynamic_cast<ReleaseStreamRtmpPacket*>(
                                conn.get());
        if (release_stream) {
            if ((ret = on_recv_release_stream(release_stream)) != SUCCESS) {
                return ret;
            }
            continue;
        }

        // publish
        auto publish = dynamic_cast<PublishRtmpPacket*>(conn.get());
        if (publish) {
            if ((ret = on_recv_publish(publish)) != SUCCESS) {
                return ret;
            }
            publishing = true;
            break;
        }
    }

    return ret;
}

error_t RtmpPreRequest::on_recv_create_stream(
        CreateStreamRtmpPacket* create_stream) const {
    error_t ret  = SUCCESS;
    ret = hook->send_result(create_stream->transaction_id, 1.0);

    if (ret != SUCCESS) {
        sp_error("Fail send __result %.*s failed, ret %d",
                  create_stream->name.av_len, create_stream->name.av_val, ret);
        return ret;
    }
    sp_debug("send __result %.*s, txn %lf, i. %d", create_stream->name.av_len,
            create_stream->name.av_val, create_stream->transaction_id, 0);
    return ret;
}

error_t RtmpPreRequest::on_recv_play(PlayRtmpPacket* play) {
    error_t ret  = SUCCESS;
    stream_params = "/" + play->stream_params;

    sp_info("Play stream %s, playing: %d",
            stream_params.c_str(), hook->get_rtmp()->m_bPlaying);

    if ((ret = hook->send_play_start(play->transaction_id)) != SUCCESS) {
        sp_error("Fail send __result %.*s failed, ret %d", play->name.av_len,
                 play->name.av_val, ret);
        return ret;
    }
    sp_trace("OK send play __result %.*s, txn %lf, i. %d",
            play->name.av_len, play->name.av_val,
            play->transaction_id, 0);

    if ((ret = hook->send_sample_access()) != SUCCESS) {
        sp_error("Fail send sample access ret %d", ret);
        return ret;
    }

    if ((ret = hook->send_stream_begin()) != SUCCESS) {
        sp_error("Fail send stream begin access ret %d", ret);
        return ret;
    }
    sp_debug("success send sample access");
    return ret;
}

error_t RtmpPreRequest::on_recv_publish(PublishRtmpPacket* publish) {
    error_t ret = SUCCESS;
    stream_params = "/" + publish->stream_params;

    sp_info("Publish stream %s, %s, playing %d",
             stream_params.c_str(), publish->publish_type.c_str(),
             hook->get_rtmp()->m_bPlaying);

    if ((ret = hook->send_publish_start(publish->transaction_id)) != SUCCESS) {
        sp_error("Fail send __result %.*s failed, ret %d",
                  publish->name.av_len, publish->name.av_val, ret);
        return ret;
    }

    sp_info("OK send publish __result %.*s, txn %lf, i. %d",
             publish->name.av_len, publish->name.av_val,
             publish->transaction_id, 0);

    if ((ret = hook->send_stream_begin()) != SUCCESS) {
        sp_error("Fail send stream begin access ret %d", ret);
        return ret;
    }

    sp_info("OK send stream begin");
    return ret;
}

error_t RtmpPreRequest::on_recv_fcpublish(FCPublishRtmpPacket* p) const {
    auto ret = hook->send_result(p->transaction_id, 0);

    if (ret != SUCCESS) {
        sp_error("Fail send __result fcpublish %d,", ret);
        return ret;
    }

    sp_info("OK send fcpublish __result");
    return ret;
}

error_t RtmpPreRequest::on_recv_release_stream(
        ReleaseStreamRtmpPacket* p) const {
    auto ret = hook->send_result(p->transaction_id, 0);

    if (ret != SUCCESS) {
        sp_error("Fail send __result releaseStream %d,", ret);
        return ret;
    }

    sp_info("OK send releaseStream __result");
    return ret;
}

}  // namespace sps

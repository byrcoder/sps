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
#include <sps_rtmp_server.hpp>

namespace sps {

// TODO: IMPL
RtmpServerHandler::RtmpServerHandler() : IPhaseHandler("rtmp-server") {
}

error_t RtmpServerHandler::handler(ConnContext &ctx) {
    return ERROR_RTMP_NOT_IMPL;
}

RtmpServer404Handler::RtmpServer404Handler() : IPhaseHandler("rtmp-404-handler") {
}

error_t RtmpServer404Handler::handler(ConnContext &ctx) {
    sp_error("host not found host: %s", ctx.req ? ctx.req->get_host() : "");
    return ERROR_UPSTREAM_NOT_FOUND;
}

RtmpPrepareHandler::RtmpPrepareHandler() : IPhaseHandler("rtmp-handshake-handler") {

}

error_t RtmpPrepareHandler::handler(ConnContext &ctx) {
    RtmpConnHandler* rt  = dynamic_cast<RtmpConnHandler*>(ctx.conn);
    error_t          ret = SUCCESS;
    auto             rtmp = rt->hk->get_rtmp();
    RtmpServerHandshake shk(rt->hk.get());

    // handshake
    if ((ret = shk.handshake()) != SUCCESS) {
        sp_error("rtmp s-handshake failed ret: %d", ret);
        return ERROR_RTMP_HANDSHAKE;
    }

    // connect
    RtmpPreRequest pre(rt->hk.get());
    if ((ret = pre.connect()) != SUCCESS) {
        sp_error("rtmp s-connect failed ret: %d", ret);
        return ret;
    }

    ctx.req = std::make_shared<RequestUrl>();
    std::string tc_url = pre.tc_url + pre.stream_params;
    if ((ret = ctx.req->parse_url(tc_url)) != SUCCESS) {
        sp_error("rtmp tc_url parse failed ret: %d, %s", ret, tc_url.c_str());
        return ret;
    }

    sp_info("rtmp conn success %s:%d", ctx.socket->get_cip().c_str(),
            ctx.socket->get_port());

    return SUCCESS;
}

RtmpServerHandshake::RtmpServerHandshake(LibRTMPHooks* r) {
    this->hook = r;
}

error_t RtmpServerHandshake::handshake() {
    return hook->server_handshake();
}

RtmpPreRequest::RtmpPreRequest(LibRTMPHooks* r) {
    this->hook = r;
}

error_t RtmpPreRequest::connect() {
    error_t   ret  = SUCCESS;

    WrapRtmpPacket   pkt;
    PIRTMPPacket     conn;

    if ((ret = hook->recv_packet(pkt)) != SUCCESS) {
        return ret;
    }

    ret = RtmpPacketDecoder::decode(pkt, conn);
    if (ret != SUCCESS) {
        sp_error("conn decoded failed ret: %d", ret);
        return ret;
    }

    auto conn_packet = dynamic_cast<ConnectRtmpPacket*>(conn.get());
    if (!conn_packet) {
        ret = ERROR_RTMP_AMF_DECODE;
        sp_error("expect amf conn ret: %d", ret);
        return ret;
    }

    tc_url = std::string(conn_packet->tc_url.av_val, conn_packet->tc_url.av_len);
    txn    = conn_packet->transaction_id;

    ret = hook->server_bandwidth();
    if (ret != SUCCESS) {
        return ret;
    }

    if (((ret = hook->send_set_chunked_size()) != SUCCESS) ||
            ((ret = hook->send_connect_result(txn)) != SUCCESS)) {
        sp_error("failed send connect result ret %d", ret);
        return ret;
    }

    int i = 0;
    while ((ret = hook->recv_packet(pkt)) == SUCCESS) {
        ret = RtmpPacketDecoder::decode(pkt, conn);

        if (ret != SUCCESS) {
            sp_error("fail decode packet type %d", pkt.packet.m_packetType);
            return ret;
        }

        {
            auto create_stream = dynamic_cast<CreateStreamRtmpPacket *>(conn.get());

            if (create_stream) {
                ret = hook->send_result(create_stream->transaction_id, ++i);

                if (ret != SUCCESS) {
                    sp_error("fail res __result %.*s failed, ret %d", create_stream->name.av_len,
                             create_stream->name.av_val, ret);
                    return ret;
                }
                sp_info("send __result %.*s, txn %lf, i. %d", create_stream->name.av_len,
                        create_stream->name.av_val, create_stream->transaction_id, i);
                continue;
            }
        }

        {
            auto play = dynamic_cast<PlayRtmpPacket *>(conn.get());
            if (play) {
                stream_params = "/" + play->stream_params;
                sp_info("play stream %s", stream_params.c_str());
                ret = hook->send_result(play->transaction_id, ++i);

                if (ret != SUCCESS) {
                    sp_error("fail res __result %.*s failed, ret %d", play->name.av_len,
                             play->name.av_val, ret);
                    return ret;
                }
                sp_info("send __result %.*s, txn %lf, i. %d", play->name.av_len,
                        play->name.av_val, play->transaction_id, i);
                break;
            }
        }
    }

    return ret;
}

}
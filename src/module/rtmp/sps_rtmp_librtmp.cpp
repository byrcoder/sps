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
// Created by byrcoder on 2021/6/24.
//

#include <sps_rtmp_librtmp.hpp>

#include <sps_log.hpp>
#include <librtmp/log.h>

namespace sps {

void librtmp_init_once() {
    static bool inited = false;
    if (inited) {
        return;
    }
    inited = true;
    RTMP_LogSetLevel(RTMP_LOGDEBUG);
}

RTMP_HOOK krtmp_hook = {
        .RTMP_Connect         = LibRTMPHooks::RTMP_Connect,
        .RTMP_TLS_Accept      = LibRTMPHooks::RTMP_TLS_Accept,
        .RTMPSockBuf_Send     = LibRTMPHooks::RTMPSockBuf_Send,
        .RTMPSockBuf_Fill     = LibRTMPHooks::RTMPSockBuf_Fill,
        .RTMPSockBuf_Close    = LibRTMPHooks::RTMPSockBuf_Close,
        .RTMP_IsConnected     = LibRTMPHooks::RTMP_IsConnected,
        .RTMP_Socket          = LibRTMPHooks::RTMP_Socket
};

LibRTMPHooks::LibRTMPHooks(PSocket io) {
    librtmp_init_once();

    this->skt  = std::move(io);
    this->rtmp = (RTMP*) malloc(sizeof(RTMP));
    error      = SUCCESS;

    hook = krtmp_hook;
    hook.rtmp_obj = (void *) this;

    RTMP* tmp = rtmp;

    RTMP_Init(rtmp);
    RTMP_Init_Hook(rtmp, &hook);

    sp_info("self->hook: %p", &hook);
    sp_info("rtmp->hook: %p", tmp->hook);

    sp_info("rtmp sockbuf hook: %p", &rtmp->m_sb);
    sp_info("rtmp sockbuf->rtmp %p", rtmp->m_sb.rtmp);
}

LibRTMPHooks::~LibRTMPHooks() {
    free(rtmp);
}

int LibRTMPHooks::RTMP_Connect(RTMP *r, RTMPPacket *cp) {
    if (!r->hook || !r->hook->rtmp_obj) {
        sp_error("rtmp  hook or rtmp obj hook is null! hook: %p.", r->hook);
        exit(-1);
    }
    LibRTMPHooks *hook = (LibRTMPHooks *) r->hook->rtmp_obj;
    if (hook->skt) return TRUE;

    sp_error("hooks socket is null!");

    hook->error = ERROR_RTMP_NOT_CONNECT;
    return FALSE;
}

int LibRTMPHooks::RTMP_TLS_Accept(RTMP *r, void *ctx) {
    if (!r->hook || !r->hook->rtmp_obj) {
        sp_error("rtmp  hook or rtmp obj hook is null! hook: %p.", r->hook);
        exit(-1);
    }
    LibRTMPHooks *hook = (LibRTMPHooks *) r->hook->rtmp_obj;

    hook->error = ERROR_RTMP_NOT_IMPL;

    sp_error("tls not impl");
    return -1;
}

int LibRTMPHooks::RTMPSockBuf_Send(RTMPSockBuf *sb, const char *buf, int len) {
    RTMP *r = (RTMP *) sb->rtmp;
    auto hook = (LibRTMPHooks *) r->hook->rtmp_obj;

    sp_info("send rtmp buf len :%d", len);
    hook->error = hook->skt->write((void *) buf, len);
    if (hook->error == SUCCESS) {
        sp_info("send success rtmp buf len :%d", len);
        return len;
    } else {
        sp_info("send failed rtmp buf len :%d, ret: %d", len, hook->error);
        return -1;
    }
}

int LibRTMPHooks::RTMPSockBuf_Fill(RTMPSockBuf *sb, char *buf, int nb_bytes) {
    RTMP *r = (RTMP *) sb->rtmp;

    if (r == nullptr || r->hook == nullptr) {
        sp_error("socketbuf's: %p rtmp or rtmp->hook is null rtmp: %p", sb, r);
        exit(-1);
    }

    auto *hook = (LibRTMPHooks *) r->hook->rtmp_obj;
    size_t nread = 0;

    hook->error = hook->skt->read(buf, nb_bytes, nread);
    if (hook->error == SUCCESS) {
        sp_info("recv rtmp success len: %lu", nread);
        return nread;
    } else {
        sp_error("recv rtmp failed ret: %d", hook->error);
        return -1;
    }
}

int LibRTMPHooks::RTMPSockBuf_Close(RTMPSockBuf *sb) {
    return 0;
}

int LibRTMPHooks::RTMP_IsConnected(RTMP *r) {
    auto *hook = (LibRTMPHooks *) r->hook->rtmp_obj;

    return hook->skt ? TRUE : FALSE;
}

int LibRTMPHooks::RTMP_Socket(RTMP *r) {
    auto *hook = (LibRTMPHooks *) r->hook->rtmp_obj;

    // ignore
    return -1;
}

bool RTMPPacketDecoder::is_set_chunked_size(int pkt_type) {
    return pkt_type == RTMP_PACKET_TYPE_CHUNK_SIZE;
}

bool RTMPPacketDecoder::is_abort_message(int pkt_type) {
    return pkt_type == RTMP_PACKET_TYPE_ABORT_STREAM;
}

bool RTMPPacketDecoder::is_ack(int pkt_type) {
    return pkt_type == RTMP_PACKET_TYPE_ACK;
}

bool RTMPPacketDecoder::is_user_control(int pkt_type) {
    return pkt_type == RTMP_PACKET_TYPE_USER_CONTROL;
}

bool RTMPPacketDecoder::is_ack_win_size(int pkt_type) {
    return pkt_type == RTMP_PACKET_TYPE_WIN_ACK_SIZE;
}

bool RTMPPacketDecoder::is_set_peer_bandwidth(int pkt_type) {
    return pkt_type == RTMP_PACKET_TYPE_SET_PEER_BANDWIDTH;
}

bool RTMPPacketDecoder::is_command(int pkt_type) {
    return pkt_type == RTMP_PACKET_TYPE_AMF0_DATA ||
            pkt_type == RTMP_PACKET_TYPE_AMF0_CMD ||
            pkt_type == RTMP_PACKET_TYPE_AMF3_DATA ||
            pkt_type == RTMP_PACKET_TYPE_AMF3_CMD;
}

error_t RTMPPacketDecoder::decode(RTMPPacket &packet, PIRTMPPacket &result) {
    int pkt_type = packet.m_packetType;
    error_t ret  = SUCCESS;

    switch (pkt_type) {
        case RTMP_PACKET_TYPE_SET_CHUNK_SIZE:

            break;
        case RTMP_PACKET_TYPE_ABORT_STREAM:

            break;
        case RTMP_PACKET_TYPE_ACK:

            break;
        case RTMP_PACKET_TYPE_USER_CONTROL:

            break;
        case RTMP_PACKET_TYPE_WIN_ACK_SIZE:

            break;
        case RTMP_PACKET_TYPE_SET_PEER_BANDWIDTH:

            break;
        case RTMP_PACKET_TYPE_AMF0_DATA:
        case RTMP_PACKET_TYPE_AMF0_CMD:
        case RTMP_PACKET_TYPE_AMF3_DATA:
        case RTMP_PACKET_TYPE_AMF3_CMD:

            break;
        case RTMP_PACKET_TYPE_VIDEOS:
        case RTMP_PACKET_TYPE_AUDIOS:

            break;
        default:
            sp_warn("ignore pkt_type: %d", pkt_type);
            return SUCCESS;
    }
    return ret;
}

}
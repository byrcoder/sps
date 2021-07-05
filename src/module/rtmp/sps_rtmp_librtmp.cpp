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

#include <librtmp/amf.h>
#include <librtmp/log.h>
#include <string>

#define SAVC(x) static const AVal av_##x = AMF_CONST_VAL_STRING(#x)

SAVC(app);
SAVC(connect);
SAVC(flashVer);
SAVC(swfUrl);
SAVC(pageUrl);
SAVC(tcUrl);
SAVC(fpad);
SAVC(capabilities);
SAVC(audioCodecs);
SAVC(videoCodecs);
SAVC(videoFunction);
SAVC(objectEncoding);
SAVC(_result);
SAVC(createStream);
SAVC(getStreamLength);
SAVC(play);
SAVC(fmsVer);
SAVC(mode);
SAVC(level);
SAVC(code);
SAVC(description);
SAVC(secureToken);
SAVC(onStatus);
SAVC(status);

namespace sps {

void librtmp_init_once() {
    static bool inited = false;
    if (inited) {
        return;
    }
    inited = true;
    RTMP_LogSetLevel(RTMP_LOGDEBUG);
}

error_t get_amf_prop(AMFObjectProperty* prop, AMFObject** obj) {
    if (prop->p_UTCoffset == AMF_INVALID) {
        return ERROR_RTMP_AMF_PROP_NOT_FOUND;
    }

    if (prop->p_type != AMF_OBJECT && prop->p_type != AMF_NULL) {
        return ERROR_RTMP_AMF_PROP_TYPE;
    }
    *obj = &prop->p_vu.p_object;
    return SUCCESS;
}

error_t get_amf_prop(AMFObjectProperty* prop, AVal* val) {
    if (prop->p_UTCoffset == AMF_INVALID) {
        return ERROR_RTMP_AMF_PROP_NOT_FOUND;
    }

    if (prop->p_type != AMF_STRING) {
        return ERROR_RTMP_AMF_PROP_TYPE;
    }
    *val = prop->p_vu.p_aval;
    return SUCCESS;
}

error_t get_amf_prop(AMFObjectProperty* prop, double* num) {
    if (prop->p_UTCoffset == AMF_INVALID) {
        return ERROR_RTMP_AMF_PROP_NOT_FOUND;
    }

    if (prop->p_type != AMF_NUMBER) {
        return ERROR_RTMP_AMF_PROP_TYPE;
    }
    *num = prop->p_vu.p_number;
    return SUCCESS;
}

error_t get_amf_object(AMFObject* obj, const char* name, AMFObject** obj_val) {
    AVal found = AMF_VAL_STRING(name);
    auto ret   = SUCCESS;

    AMFObjectProperty* prop = AMF_GetProp(obj, &found, -1);
    return get_amf_prop(prop, obj_val);
}

error_t get_amf_string(AMFObject* obj, const char* name, AVal* str_val) {
    AVal found = AMF_VAL_STRING(name);
    auto ret   = SUCCESS;

    AMFObjectProperty* prop = AMF_GetProp(obj, &found, -1);
    return get_amf_prop(prop, str_val);
}

error_t get_amf_num(AMFObject* obj, const char* name, double* num) {
    AVal found = AMF_VAL_STRING(name);
    auto ret   = SUCCESS;

    AMFObjectProperty* prop = AMF_GetProp(obj, &found, -1);
    return get_amf_prop(prop, num);
}

// work as librtmp
char* AMF_EncodeNull(char* start) {
    *start = 0x05;
    return start+1;
}

char* AMF_EncodeStartObject(char* start) {
    *start = 0x03;
    return start+1;
}

char* AMF_EncodeEndObject(char* start) {
    *start++ = 0x00;
    *start++ = 0x00;
    *start++ = 0x09;
    return start;
}

bool equal_val(AVal* src, const char* c) {
    return strlen(c) == src->av_len && memcmp(src->av_val, c, src->av_len) == 0;
}

WrapRtmpPacket::WrapRtmpPacket(bool own) {
    this->own = own;
}

WrapRtmpPacket::~WrapRtmpPacket() {
    reset();
}

void WrapRtmpPacket::reset() {
    if (own) {
        RTMPPacket_Free(&packet);
    }
}

bool WrapRtmpPacket::is_video() {
    return packet.m_body && packet.m_packetType == RTMP_PACKET_TYPE_VIDEO;
}

bool WrapRtmpPacket::is_audio() {
    return packet.m_body && packet.m_packetType == RTMP_PACKET_TYPE_AUDIO;
}

bool WrapRtmpPacket::is_script() {
    return packet.m_body && (packet.m_packetType == RTMP_PACKET_TYPE_AMF0_DATA
        || packet.m_packetType == RTMP_PACKET_TYPE_AMF3_DATA);
}

uint8_t* WrapRtmpPacket::data() {
    return (uint8_t*) packet.m_body;
}

size_t WrapRtmpPacket::size() {
    return packet.m_nBodySize;
}

RTMP_HOOK krtmp_hook = {
        .RTMP_Connect         = RtmpHook::SPS_RTMP_Connect,
        .RTMP_TLS_Accept      = RtmpHook::SPS_RTMP_TLS_Accept,
        .RTMPSockBuf_Send     = RtmpHook::SPS_RTMPSockBuf_Send,
        .RTMPSockBuf_Fill     = RtmpHook::SPS_RTMPSockBuf_Fill,
        .RTMPSockBuf_Close    = RtmpHook::SPS_RTMPSockBuf_Close,
        .RTMP_IsConnected     = RtmpHook::SPS_RTMP_IsConnected,
        .RTMP_Socket          = RtmpHook::SPS_RTMP_Socket
};

RtmpHook::RtmpHook(PSocket io)  {
    librtmp_init_once();

    this->skt  = std::move(io);
    this->rtmp = (RTMP*) malloc(sizeof(RTMP));
    error      = SUCCESS;

    hook = krtmp_hook;
    hook.rtmp_obj = (void *) this;

    rtmp->m_nServerBW    = 200 * 1000 * 1000;

    RTMP_Init(rtmp);
    RTMP_Init_Hook(rtmp, &hook);
    rtmp->m_outChunkSize = 128;  // TODO: fixme for config
    rtmp->m_inChunkSize  = 128;

    sp_info("self->hook: %p", &hook);
    sp_info("rtmp sockbuf hook: %p", &rtmp->m_sb);
    sp_info("rtmp sockbuf->rtmp %p", rtmp->m_sb.rtmp);
}

RtmpHook::~RtmpHook() {
    RTMP_Close(rtmp);
    free(rtmp);
}

void RtmpHook::set_recv_timeout(utime_t tm) {
    if (skt) {
        skt->set_recv_timeout(tm);
    }
}

void RtmpHook::set_send_timeout(utime_t tm) {
    if (skt) {
        skt->set_send_timeout(tm);
    }
}

error_t RtmpHook::server_handshake() {
    if (RTMP_Serve(rtmp) == FALSE) {
        sp_error("rtmp s-handshake failed ret: %d", ERROR_RTMP_HANDSHAKE);
        return error ? error : ERROR_RTMP_HANDSHAKE;
    }
    return SUCCESS;
}

error_t RtmpHook::send_ack_window_size() {
    RTMPPacket packet;
    char pbuf[256], *pend = pbuf + sizeof(pbuf);
    error_t    ret = SUCCESS;

    packet.m_nChannel = RTMP_PACKET_STREAM_ID_CONTROL;	/* control channel (invoke) */
    packet.m_headerType = 0;
    packet.m_packetType = RTMP_PACKET_TYPE_WIN_ACK_SIZE;
    packet.m_nTimeStamp = 0;
    packet.m_nInfoField2 = 0;
    packet.m_hasAbsTimestamp = 0;
    packet.m_body = pbuf + RTMP_MAX_HEADER_SIZE;

    packet.m_nBodySize = 4;

    AMF_EncodeInt32(packet.m_body, pend, 500*1000);

    if ((ret = send_packet(packet, false)) != SUCCESS) {
        sp_error("fail send ack win size ret: %d", ret);
        return ret;
    }
    sp_info("success send ack win size");
    return SUCCESS;
}

error_t RtmpHook::send_client_bandwidth() {
    RTMPPacket packet;
    char pbuf[256], *pend = pbuf + sizeof(pbuf);
    error_t    ret = SUCCESS;

    packet.m_nChannel = RTMP_PACKET_STREAM_ID_CONTROL;	/* control channel (invoke) */
    packet.m_headerType = 0;
    packet.m_packetType = RTMP_PACKET_TYPE_CLIENT_BW;
    packet.m_nTimeStamp = 0;
    packet.m_nInfoField2 = 0;
    packet.m_hasAbsTimestamp = 0;
    packet.m_body = pbuf + RTMP_MAX_HEADER_SIZE;

    packet.m_nBodySize = 5;

    char *enc     = packet.m_body;
    enc = AMF_EncodeInt32(enc, pend, rtmp->m_nClientBW);
    *enc = 0x02;

    if ((ret = send_packet(packet, false)) != SUCCESS) {
        sp_error("fail send chunked size %d, size: %d, ret: %d",
                 rtmp->m_inChunkSize, packet.m_nBodySize, ret);
        return ret;
    }
    sp_info("success amf set peer: %d, size: %d", rtmp->m_inChunkSize, packet.m_nBodySize);
    return SUCCESS;
}

error_t RtmpHook::send_set_chunked_size() {
    RTMPPacket packet   = {0};
    char pbuf[64], *pend = pbuf+sizeof(pbuf);
    AMFObject obj;
    AMFObjectProperty p, op;
    AVal av;
    error_t   ret  = SUCCESS;

    packet.m_nChannel = RTMP_PACKET_STREAM_ID_CONTROL;
    packet.m_headerType = 0;
    packet.m_packetType = RTMP_PACKET_TYPE_CHUNK_SIZE;
    packet.m_nTimeStamp = 0;
    packet.m_nInfoField2 = 0;
    packet.m_hasAbsTimestamp = 0;
    packet.m_body = pbuf + RTMP_MAX_HEADER_SIZE;
    char *enc     = packet.m_body;

    const int chunk_size = 4096;
    enc = AMF_EncodeInt32(enc, pend, chunk_size);
    packet.m_nBodySize = enc - packet.m_body;

    if ((ret = send_packet(packet, false)) != SUCCESS) {
        sp_error("fail send chunked size %d, size: %d, ret: %d",
                rtmp->m_inChunkSize, packet.m_nBodySize, ret);
        return ret;
    }

    rtmp->m_outChunkSize = rtmp->m_inChunkSize = chunk_size;

    sp_info("success amf set chunked size: %d, size: %d", rtmp->m_inChunkSize, packet.m_nBodySize);
    return SUCCESS;
}

error_t RtmpHook::send_play_start(int /** txn **/) {
    RTMPPacket packet   = {0};
    char pbuf[512], *pend = pbuf+sizeof(pbuf);
    AMFObject obj;
    AMFObjectProperty p, op;
    AVal av;
    error_t   ret  = SUCCESS;

    packet.m_nChannel = RTMP_PACKET_STREAM_ID_NET_CONNECT;
    packet.m_headerType = 0;
    packet.m_packetType = RTMP_PACKET_TYPE_INVOKE;
    packet.m_nTimeStamp = 0;
    packet.m_nInfoField2 = 0;
    packet.m_hasAbsTimestamp = 0;
    packet.m_body = pbuf + RTMP_MAX_HEADER_SIZE;
    char *enc     = packet.m_body;

    enc = AMF_EncodeString(enc, pend, &av_onStatus);
    enc = AMF_EncodeNumber(enc, pend, 0);
    enc = AMF_EncodeNull(enc);

    {
        enc = AMF_EncodeStartObject(enc);

        AMF_INIT_VAL_STRING(av,"NetStream.Play.Start");
        enc = AMF_EncodeNamedString(enc, pend, &av_level, &av_status);
        enc = AMF_EncodeNamedString(enc, pend, &av_code, &av);
        AMF_INIT_VAL_STRING(av,"Start live");
        enc = AMF_EncodeNamedString(enc, pend, &av_description, &av);

        enc = AMF_EncodeEndObject(enc);
    }

    packet.m_nBodySize = enc - packet.m_body;

    if ((ret = send_packet(packet, false)) != SUCCESS) {
        sp_error("fail send onStatus(NetStream.Play.Start)");
        return ret;
    }

    return ret;
}

error_t RtmpHook::send_publish_start(int transaction_id) {
    RTMPPacket packet   = {0};
    char pbuf[512], *pend = pbuf+sizeof(pbuf);
    AMFObject obj;
    AMFObjectProperty p, op;
    AVal av;
    error_t   ret  = SUCCESS;

    packet.m_nChannel = RTMP_PACKET_STREAM_ID_NET_CONNECT;
    packet.m_headerType = 0;
    packet.m_packetType = RTMP_PACKET_TYPE_INVOKE;
    packet.m_nTimeStamp = 0;
    packet.m_nInfoField2 = 0;
    packet.m_hasAbsTimestamp = 0;
    packet.m_body = pbuf + RTMP_MAX_HEADER_SIZE;
    char *enc     = packet.m_body;

    enc = AMF_EncodeString(enc, pend, &av_onStatus);
    enc = AMF_EncodeNumber(enc, pend, transaction_id);
    enc = AMF_EncodeNull(enc);

    {
        enc = AMF_EncodeStartObject(enc);

        AMF_INIT_VAL_STRING(av,"NetStream.Publish.Start");
        enc = AMF_EncodeNamedString(enc, pend, &av_level, &av_status);
        enc = AMF_EncodeNamedString(enc, pend, &av_code, &av);
        AMF_INIT_VAL_STRING(av,"Start publishing");
        enc = AMF_EncodeNamedString(enc, pend, &av_description, &av);

        enc = AMF_EncodeEndObject(enc);
    }

    packet.m_nBodySize = enc - packet.m_body;

    if ((ret = send_packet(packet, false)) != SUCCESS) {
        sp_error("fail send onStatus(NetStream.Play.Start)");
        return ret;
    }

    return ret;
}

error_t RtmpHook::send_sample_access() {
    RTMPPacket packet   = {0};
    char pbuf[512], *pend = pbuf+sizeof(pbuf);
    AMFObject obj;
    AMFObjectProperty p, op;
    AVal av;
    error_t   ret  = SUCCESS;

    packet.m_nChannel = RTMP_PACKET_STREAM_ID_DATA;
    packet.m_headerType = 0;
    packet.m_packetType = RTMP_PACKET_TYPE_INFO;
    packet.m_nTimeStamp = 0;
    packet.m_nInfoField2 = 0;
    packet.m_hasAbsTimestamp = 0;
    packet.m_body = pbuf + RTMP_MAX_HEADER_SIZE;
    char *enc     = packet.m_body;

    AMF_INIT_VAL_STRING(av, "|RtmpSampleAccess");
    enc = AMF_EncodeString(enc, pend, &av);
    enc = AMF_EncodeBoolean(enc, pend, 1);
    enc = AMF_EncodeBoolean(enc, pend, 1);

    packet.m_nBodySize = enc - packet.m_body;

    if ((ret = send_packet(packet, false)) != SUCCESS) {
        sp_error("fail send |RtmpSampleAccess ret %d", ret);
        return ret;
    }

    return ret;
}

error_t RtmpHook::send_stream_begin() {
    RTMPPacket packet   = {0};
    char pbuf[512], *pend = pbuf+sizeof(pbuf);
    AMFObject obj;
    AMFObjectProperty p, op;
    AVal av;
    error_t   ret  = SUCCESS;

    packet.m_nChannel = RTMP_PACKET_STREAM_ID_CONTROL;
    packet.m_headerType = 0;
    packet.m_packetType = RTMP_PACKET_TYPE_CONTROL;
    packet.m_nTimeStamp = 0;
    packet.m_nInfoField2 = 0;
    packet.m_hasAbsTimestamp = 0;
    packet.m_body = pbuf + RTMP_MAX_HEADER_SIZE;
    char *enc     = packet.m_body;
    enc = AMF_EncodeInt16(enc, pend, 0); // stream_begin
    enc = AMF_EncodeInt32(enc, pend, 1); // stream_id

    packet.m_nBodySize = enc - packet.m_body;

    if ((ret = send_packet(packet, false)) != SUCCESS) {
        sp_error("fail send |RtmpSampleAccess ret %d", ret);
        return ret;
    }

    rtmp->m_stream_id = 1;

    return ret;
}

error_t RtmpHook::send_connect_result(double txn) {
    RTMPPacket packet   = {0};
    char pbuf[512] = { 0 };
    char *pend = pbuf + sizeof(pbuf);
    AMFObject obj;
    AMFObjectProperty p, op;
    AVal av;
    error_t   ret  = SUCCESS;

    packet.m_nChannel = RTMP_PACKET_STREAM_ID_NET_CONNECT;  // control channel (invoke) chunked id
    packet.m_headerType = 0;
    packet.m_packetType = RTMP_PACKET_TYPE_INVOKE;
    packet.m_nTimeStamp = 0;
    packet.m_nInfoField2 = 0;
    packet.m_hasAbsTimestamp = 1;
    packet.m_body = pbuf + RTMP_MAX_HEADER_SIZE;

    if (txn != 1) {
        sp_warn("result connect txn %lf not 1", txn);
    }
    char *enc = packet.m_body;
    enc = AMF_EncodeString(enc, pend, &av__result);
    enc = AMF_EncodeNumber(enc, pend, txn);
    *enc++ = AMF_OBJECT;

    AMF_INIT_VAL_STRING(av, "FMS/3,5,1,525");
    enc = AMF_EncodeNamedString(enc, pend, &av_fmsVer, &av);
    enc = AMF_EncodeNamedNumber(enc, pend, &av_capabilities, 31.0);
    // enc = AMF_EncodeNamedNumber(enc, pend, &av_mode, 1.0);
    *enc++ = 0;
    *enc++ = 0;
    *enc++ = AMF_OBJECT_END;
    *enc++ = AMF_OBJECT;

    AMF_INIT_VAL_STRING(av, "status");
    enc = AMF_EncodeNamedString(enc, pend, &av_level, &av);

    AMF_INIT_VAL_STRING(av, "NetConnection.Connect.Success");
    enc = AMF_EncodeNamedString(enc, pend, &av_code, &av);

    // TODO: FIXME WHY WROTE FAILED?
    /*
    AMF_INIT_VAL_STRING(av, "Connection succeeded.");
    enc = AMF_EncodeNamedString(enc, pend, &av_description, &av);
     */

    AMF_INIT_VAL_STRING(av, "Connection succeeded.");
    AVal name = {(char*) "description", (int) strlen("description")};
    av        = {     (char*) "Connection succeeded.", (int)  strlen("Connection succeeded.")  };
    // enc = AMF_EncodeNamedString(enc, pend, &name, &av);

    sp_info("av_code: %.*s(%d),  av_description: %d, %s, "
            "av: %s(%d)",
            av_code.av_len, av_code.av_val, av_code.av_len,
            av_description.av_len, av_description.av_val,
            av.av_val, av.av_len);

    *enc++ = 0;
    *enc++ = 0;
    *enc++ = AMF_OBJECT_END;

    packet.m_nBodySize = enc - packet.m_body;

    if ((ret = send_packet(packet, false)) != SUCCESS) {
        return ret;
    }

    return ret;
}

error_t RtmpHook::send_result(double txn, double id) {
    RTMPPacket packet;
    char pbuf[256], *pend = pbuf+sizeof(pbuf);

    packet.m_nChannel = RTMP_PACKET_STREAM_ID_NET_CONNECT;
    packet.m_headerType = 0;
    packet.m_packetType = RTMP_PACKET_TYPE_INVOKE;
    packet.m_nTimeStamp = 0;
    packet.m_nInfoField2 = 0;
    packet.m_hasAbsTimestamp = 0;
    packet.m_body = pbuf + RTMP_MAX_HEADER_SIZE;

    char *enc = packet.m_body;
    enc = AMF_EncodeString(enc, pend, &av__result);
    enc = AMF_EncodeNumber(enc, pend, txn);
    *enc++ = AMF_NULL;
    enc = AMF_EncodeNumber(enc, pend, id);

    packet.m_nBodySize = enc - packet.m_body;

    RTMPPacket_Dump(&packet);

    return send_packet(packet, false);
}

error_t RtmpHook::recv_packet(WrapRtmpPacket &pkt) {
    error_t ret = SUCCESS;

    pkt.reset();

    do {
        if (RTMP_ReadPacket(rtmp, &pkt.packet) != TRUE) {
            ret = error;
            sp_error("rtmp recv failed ret: %d", ret);
            return ret;
        }

        if (RTMPPacket_IsReady(&pkt.packet)) {
            RTMP_ClientPacket(rtmp, &pkt.packet);
            break;
        }
    } while(true);

    {
        auto& packet = pkt.packet;
        sp_debug("rtmp packet head_type: %u, m_packetType: %u, "
                "m_hasAbsTimestamp: %u, m_nChannel: %d, "
                "m_nTimeStamp: %u, m_nInfoField2: %d, "
                "m_nBodySize: %u, m_nBytesRead: %u",
                packet.m_headerType, packet.m_packetType,
                packet.m_hasAbsTimestamp, packet.m_nChannel,
                packet.m_nTimeStamp, packet.m_nInfoField2,
                packet.m_nBodySize, packet.m_nBytesRead);
    }
    return ret;
}

error_t RtmpHook::send_packet(RTMPPacket& pkt, bool queue) {
    if (RTMP_SendPacket(rtmp, &pkt, queue ? 1 : 0) != TRUE) {
        sp_error("send packet failed error %d", error);
        return error;
    }
    return SUCCESS;
}

error_t RtmpHook::client_connect(const std::string& url, const std::string& params, bool publish) {
    error_t ret = SUCCESS;

    if (RTMP_SetupURL(rtmp, (char*) (url + " " + params).c_str()) == FALSE) {
        sp_error("fail set url %s.", url.c_str());
        return ERROR_RTMP_CONNECT;
    }

    if (publish) {
        RTMP_EnableWrite(rtmp);
    }

    if (!RTMP_Connect(rtmp, NULL)) {
        sp_error("fail connect url %s. error %d", url.c_str(), ret);
        return error != SUCCESS ? error : ERROR_RTMP_CONNECT;
    }

    if (!RTMP_ConnectStream(rtmp, 0)) {
        sp_error("fail create stream url %s. error %d", url.c_str(), ret);
        return error != SUCCESS ? error : ERROR_RTMP_CONNECT;
    }

    sp_info("success connect %s", url.c_str());
    return ret;
}

error_t RtmpHook::send_server_bandwidth() {
    RTMPPacket packet;
    char pbuf[256], *pend = pbuf + sizeof(pbuf);
    error_t    ret = SUCCESS;

    packet.m_nChannel = RTMP_PACKET_STREAM_ID_CONTROL;	/* control channel (invoke) */
    packet.m_headerType = 0;
    packet.m_packetType = RTMP_PACKET_TYPE_SERVER_BW;
    packet.m_nTimeStamp = 0;
    packet.m_nInfoField2 = 0;
    packet.m_hasAbsTimestamp = 0;
    packet.m_body = pbuf + RTMP_MAX_HEADER_SIZE;

    packet.m_nBodySize = 5;

    char *enc     = packet.m_body;
    enc = AMF_EncodeInt32(enc, pend, rtmp->m_nServerBW);
    *enc = 0x02; // dynamic limit

    if ((ret = send_packet(packet, false)) != SUCCESS) {
        sp_error("fail send chunked size %d, size: %d, ret: %d",
                 rtmp->m_inChunkSize, packet.m_nBodySize, ret);
        return ret;
    }
    sp_info("success amf set peer: %d, size: %d", rtmp->m_inChunkSize, packet.m_nBodySize);
    return SUCCESS;
}

error_t RtmpHook::send_connect(const std::string& app, const std::string& tc_url) {
    RTMPPacket packet;
    char pbuf[256], *pend = pbuf+sizeof(pbuf);
    AVal av;
    error_t ret = SUCCESS;

    packet.m_nChannel   = RTMP_PACKET_STREAM_ID_NET_CONNECT;     // stream_id 0x03
    packet.m_headerType = 0;
    packet.m_packetType = RTMP_PACKET_TYPE_INVOKE;
    packet.m_nTimeStamp = 0;
    packet.m_nInfoField2 = 0;
    packet.m_hasAbsTimestamp = 0;
    packet.m_body = pbuf + RTMP_MAX_HEADER_SIZE;

    char* enc = packet.m_body;
    enc = AMF_EncodeString(enc, pend, &av_connect); // command
    enc = AMF_EncodeNumber(enc, pend, ++transaction_id);    // transaction_id

    // object
    enc = AMF_EncodeStartObject(enc);

    // app
    AMF_INIT_VAL_STRING(av, app.c_str());
    enc = AMF_EncodeNamedString(enc, pend, &av_app, &av);

    // flash ver
    AMF_INIT_VAL_STRING(av, "LNX 9,0,124,2"); // ffmpeg flashVer
    enc = AMF_EncodeNamedString(enc, pend, &av_flashVer, &av);

    // tc_url
    AMF_INIT_VAL_STRING(av, tc_url.c_str());
    enc = AMF_EncodeNamedString(enc, pend, &av_tcUrl, &av);

    // fpad, true if proxy
    enc = AMF_EncodeNamedBoolean(enc, pend, &av_fpad, 0);

    enc = AMF_EncodeNamedNumber(enc, pend, &av_capabilities, 15);

    enc = AMF_EncodeNamedNumber(enc, pend, &av_audioCodecs, 4071); // ffmpeg support

    enc = AMF_EncodeNamedNumber(enc, pend, &av_videoCodecs, 252); // ffmpeg support

    enc = AMF_EncodeNamedNumber(enc, pend, &av_objectEncoding, 0); // default amf0

    enc = AMF_EncodeEndObject(enc);

    packet.m_nBodySize = enc - packet.m_body;

    if ((ret = send_packet(packet, false)) != SUCCESS) {
        sp_error("fail send connect ret %d", ret);
        return ret;
    }

    sp_info("success send connect ret %d", ret);
    return ret;
}

error_t RtmpHook::create_stream() {
    RTMPPacket packet;
    char pbuf[256], *pend = pbuf+sizeof(pbuf);
    AVal av;
    error_t ret = SUCCESS;

    packet.m_nChannel   = RTMP_PACKET_STREAM_ID_NET_CONNECT;     // stream_id 0x03
    packet.m_headerType = 0;
    packet.m_packetType = RTMP_PACKET_TYPE_INVOKE;
    packet.m_nTimeStamp = 0;
    packet.m_nInfoField2 = 0;
    packet.m_hasAbsTimestamp = 0;
    packet.m_body = pbuf + RTMP_MAX_HEADER_SIZE;

    char* enc = packet.m_body;
    enc = AMF_EncodeString(enc, pend, &av_createStream); // command
    enc = AMF_EncodeNumber(enc, pend, ++transaction_id);    // transaction_id
    enc = AMF_EncodeNull(enc);

    packet.m_nBodySize = enc - packet.m_body;

    if ((ret = send_packet(packet, false)) != SUCCESS) {
        sp_error("fail create stream ret %d", ret);
        return ret;
    }

    sp_info("success create stream ret %d", ret);
    return ret;
}

error_t RtmpHook::send_buffer_length() {
    RTMPPacket packet   = {0};
    char pbuf[64], *pend = pbuf+sizeof(pbuf);
    AMFObject obj;
    AMFObjectProperty p, op;
    AVal av;
    error_t   ret  = SUCCESS;

    packet.m_nChannel = RTMP_PACKET_STREAM_ID_CONTROL;
    packet.m_headerType = 0;
    packet.m_packetType = RTMP_PACKET_TYPE_CONTROL;
    packet.m_nTimeStamp = 0;
    packet.m_nInfoField2 = 0;
    packet.m_hasAbsTimestamp = 0;
    packet.m_body = pbuf + RTMP_MAX_HEADER_SIZE;
    char *enc     = packet.m_body;

    return ERROR_RTMP_NOT_IMPL;
}

int RtmpHook::SPS_RTMP_Connect(RTMP* r, RTMPPacket *cp /** cp* */) {
    if (!r->hook || !r->hook->rtmp_obj) {
        sp_error("rtmp  hook or rtmp obj hook is null! hook: %p.", r->hook);
        exit(-1);
    }
    auto *hook = (RtmpHook *) r->hook->rtmp_obj;
    if (hook->skt) return TRUE;

    sp_error("hooks socket is null!");

    hook->error = ERROR_RTMP_NOT_CONNECT;
    return FALSE;
}

int RtmpHook::SPS_RTMP_TLS_Accept(RTMP *r, void *ctx /** ctx **/) {
    if (!r->hook || !r->hook->rtmp_obj) {
        sp_error("rtmp  hook or rtmp obj hook is null! hook: %p.", r->hook);
        exit(-1);
    }
    auto *hook = (RtmpHook *) r->hook->rtmp_obj;

    hook->error = ERROR_RTMP_NOT_IMPL;

    sp_error("tls not impl");
    return -1;
}

int RtmpHook::SPS_RTMPSockBuf_Send(RTMPSockBuf *sb, const char *buf, int len) {
    RTMP *r = (RTMP *) sb->rtmp;
    auto hook = (RtmpHook *) r->hook->rtmp_obj;

    sp_debug("send rtmp buf len :%d", len);
    hook->error = hook->skt->write((void *) buf, len);
    if (hook->error == SUCCESS) {
        sp_debug("send success rtmp buf len :%d", len);
        return len;
    } else {
        sp_error("send failed rtmp buf len %d, ret %d", len, hook->error);
        return -1;
    }
}

int RtmpHook::SPS_RTMPSockBuf_Fill(RTMPSockBuf *sb, char *buf, int nb_bytes) {
    RTMP *r = (RTMP *) sb->rtmp;

    if (r == nullptr || r->hook == nullptr) {
        sp_error("socketbuf's: %p rtmp or rtmp->hook is null rtmp: %p", sb, r);
        exit(-1);
    }

    auto *hook = (RtmpHook *) r->hook->rtmp_obj;
    size_t nread = 0;

    hook->error = hook->skt->read(buf, nb_bytes, nread);
    if (hook->error == SUCCESS) {
        sp_debug("recv rtmp success len: %lu", nread);
        return nread;
    } else {
        sp_error("recv rtmp failed ret: %d", hook->error);
        return -1;
    }
}

int RtmpHook::SPS_RTMPSockBuf_Close(RTMPSockBuf *sb /** sb **/) {
    return 0;
}

int RtmpHook::SPS_RTMP_IsConnected(RTMP *r) {
    auto *hook = (RtmpHook *) r->hook->rtmp_obj;

    return hook->skt ? TRUE : FALSE;
}

int RtmpHook::SPS_RTMP_Socket(RTMP *r /** r **/) {
    // auto *hook = (LibRTMPHooks *) r->hook->rtmp_obj;
    // ignore
    return -1;
}

}
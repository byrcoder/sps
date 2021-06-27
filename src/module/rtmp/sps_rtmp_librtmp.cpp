#include <iso646.h>
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

#include <sstream>
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

bool equal_val(AVal* src, const char* c) {
    return strlen(c) == src->av_len && memcmp(src->av_val, c, src->av_len) == 0;
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

LibRTMPHooks::LibRTMPHooks(PSocket io)  {
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

LibRTMPHooks::~LibRTMPHooks() {
    free(rtmp);
}

error_t LibRTMPHooks::server_handshake() {
    if (RTMP_Serve(rtmp) == FALSE) {
        sp_error("rtmp s-handshake failed ret: %d", ERROR_RTMP_HANDSHAKE);
        return error ? error : ERROR_RTMP_HANDSHAKE;
    }
    return SUCCESS;
}

error_t LibRTMPHooks::server_bandwidth() {
    if (RTMP_SendServerBW(rtmp) == FALSE) {
        sp_error("fail send err %d", error);
        return error;
    }
    return error;
}

error_t LibRTMPHooks::send_set_chunked_size() {
    RTMPPacket packet   = {0};
    char pbuf[64], *pend = pbuf+sizeof(pbuf);
    AMFObject obj;
    AMFObjectProperty p, op;
    AVal av;
    error_t   ret  = SUCCESS;

    packet.m_nChannel = 0x02;
    packet.m_headerType = 0;
    packet.m_packetType = RTMP_PACKET_TYPE_CHUNK_SIZE;
    packet.m_nTimeStamp = 0;
    packet.m_nInfoField2 = 0;
    packet.m_hasAbsTimestamp = 0;
    packet.m_body = pbuf + RTMP_MAX_HEADER_SIZE;
    char *enc     = packet.m_body;

    enc = AMF_EncodeInt32(enc, pend, 4096);
    packet.m_nBodySize = enc - packet.m_body;

    if ((ret = send_packet(packet, false)) != SUCCESS) {
        sp_error("fail send chunked size %d, size: %d, ret: %d",
                rtmp->m_inChunkSize, packet.m_nBodySize, ret);
        return ret;
    }
    sp_info("success amf set chunked size: %d, size: %d", rtmp->m_inChunkSize, packet.m_nBodySize);
    return SUCCESS;
}


error_t LibRTMPHooks::send_connect_result(double txn) {
    RTMPPacket packet   = {0};
    char pbuf[512] = { 0 };
    char *pend = pbuf + sizeof(pbuf);
    AMFObject obj;
    AMFObjectProperty p, op;
    AVal av;
    error_t   ret  = SUCCESS;

    packet.m_nChannel = 0x03;  // control channel (invoke) chunked id
    packet.m_headerType = 0;
    packet.m_packetType = RTMP_PACKET_TYPE_INVOKE;
    packet.m_nTimeStamp = 0;
    packet.m_nInfoField2 = 0;
    packet.m_hasAbsTimestamp = 1;
    packet.m_body = pbuf + RTMP_MAX_HEADER_SIZE;

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

error_t  LibRTMPHooks::send_result(double txn, double id) {
    RTMPPacket packet;
    char pbuf[256], *pend = pbuf+sizeof(pbuf);

    packet.m_nChannel = 0x03;     // control channel (invoke)
    packet.m_headerType = 0; /* RTMP_PACKET_SIZE_MEDIUM; */
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

error_t LibRTMPHooks::recv_packet(WrapRtmpPacket &pkt) {
    error_t ret = SUCCESS;

    pkt.reset();

    do {
        if (RTMP_ReadPacket(rtmp, &pkt.packet) != TRUE) {
            ret = error;
            sp_error("rtmp recv failed ret: %d", ret);
            return ret;
        }

        if (RTMPPacket_IsReady(&pkt.packet)) {
            break;
        }
    } while(true);

    {
        auto& packet = pkt.packet;
        sp_info("rtmp packet head_type: %u, m_packetType: %u, "
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

error_t LibRTMPHooks::send_packet(RTMPPacket& pkt, bool queue) {
    if (RTMP_SendPacket(rtmp, &pkt, queue ? 1 : 0) != TRUE) {
        sp_error("send packet failed error %d", error);
        return error;
    }
    return SUCCESS;
}

int LibRTMPHooks::RTMP_Connect(RTMP* r, RTMPPacket* /** cp* */) {
    if (!r->hook || !r->hook->rtmp_obj) {
        sp_error("rtmp  hook or rtmp obj hook is null! hook: %p.", r->hook);
        exit(-1);
    }
    auto *hook = (LibRTMPHooks *) r->hook->rtmp_obj;
    if (hook->skt) return TRUE;

    sp_error("hooks socket is null!");

    hook->error = ERROR_RTMP_NOT_CONNECT;
    return FALSE;
}

int LibRTMPHooks::RTMP_TLS_Accept(RTMP *r, void* /** ctx **/) {
    if (!r->hook || !r->hook->rtmp_obj) {
        sp_error("rtmp  hook or rtmp obj hook is null! hook: %p.", r->hook);
        exit(-1);
    }
    auto *hook = (LibRTMPHooks *) r->hook->rtmp_obj;

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

int LibRTMPHooks::RTMPSockBuf_Close(RTMPSockBuf* /** sb **/) {
    return 0;
}

int LibRTMPHooks::RTMP_IsConnected(RTMP *r) {
    auto *hook = (LibRTMPHooks *) r->hook->rtmp_obj;

    return hook->skt ? TRUE : FALSE;
}

int LibRTMPHooks::RTMP_Socket(RTMP* /** r **/) {
    // auto *hook = (LibRTMPHooks *) r->hook->rtmp_obj;
    // ignore
    return -1;
}

WrapRtmpPacket::~WrapRtmpPacket() {
    reset();
}

void WrapRtmpPacket::reset() {
    RTMPPacket_Free(&packet);
}

AmfRtmpPacket::AmfRtmpPacket() {
    AMF_Reset(&amf_object);
}

AmfRtmpPacket::~AmfRtmpPacket() {
    AMF_Reset(&amf_object);
}

static std::string debug_amf_object(struct AMFObjectProperty *amf) {
    std::stringstream ss;
    ss << "amf_name: " << std::string(amf->p_name.av_val, amf->p_name.av_len) << "\t"
       <<"amf_type: " << amf->p_type << "\t";
    if (amf->p_type == AMF_NUMBER) {
        ss << amf->p_vu.p_number;
    } else if (amf->p_type == AMF_STRING) {
        ss << std::string(amf->p_vu.p_aval.av_val, amf->p_vu.p_aval.av_len);
    } else if (AMF_OBJECT == amf->p_type) {
        ss << "{object}";
    } else {
        ss << "others";
    }
    ss << "\t";
    return ss.str();
}

error_t AmfRtmpPacket::decode(WrapRtmpPacket& pkt) {
    error_t ret  = SUCCESS;
    auto& packet = pkt.packet;

    sp_info ("amf type: %d", pkt.packet.m_packetType);

    if (RtmpPacketDecoder::is_amf3_command(pkt.packet.m_packetType)) {
        ret = AMF3_Decode(&amf_object, packet.m_body, packet.m_nBodySize, 0);
    } else {
        ret = AMF_Decode(&amf_object, packet.m_body, packet.m_nBodySize, 0);
    }

    if (ret < 0) {
        ret = ERROR_RTMP_AMF_DECODE;
        sp_error("decode amf failed type: %d,  %.*s, %X, %X",
                packet.m_packetType, packet.m_nBodySize, packet.m_body,
                 *packet.m_body, *(packet.m_body+1));
        return ret;
    }

    sp_info ("amf num: %d", amf_object.o_num);
    for (int i = 0; i < amf_object.o_num; ++i) {
        auto amf = amf_object.o_props+i;
        sp_info("%d. %s", i, debug_amf_object(amf).c_str());
    }
    return SUCCESS;
}

error_t AmfRtmpPacket::convert_self(PIRTMPPacket& result) {
    if (amf_object.o_num <= 0) {
        sp_error("amf not object num %d <= 0", amf_object.o_num);
        return ERROR_RTMP_AMF_CMD_CONVERT;
    }

    AVal name = {0, 0};
    auto ret  = get_amf_prop(amf_object.o_props, &name);
    if (ret != SUCCESS) {
        sp_error("amf first object: %d not string", amf_object.o_props->p_type);
        return ret;
    }

    sp_info("amf name: %.*s, %d", name.av_len, name.av_val, name.av_len);
    if (equal_val(&name, "connect")) {
        auto conn = std::make_unique<ConnectRtmpPacket>();

        if (conn->from(amf_object) != SUCCESS) {
            return ERROR_RTMP_AMF_CMD_CONVERT;
        }

        result = std::move(conn);
    } else if (equal_val(&name, "createStream")) {
        auto conn = std::make_unique<CreateStreamRtmpPacket>();

        if (conn->from(amf_object) != SUCCESS) {
            return ERROR_RTMP_AMF_CMD_CONVERT;
        }

        result = std::move(conn);
    } else if (equal_val(&name, "play")){
        auto conn = std::make_unique<PlayRtmpPacket>();

        if (conn->from(amf_object) != SUCCESS) {
            return ERROR_RTMP_AMF_CMD_CONVERT;
        }

        result = std::move(conn);
    }

    return SUCCESS;
}

error_t CommandRtmpPacket::decode(WrapRtmpPacket &packet) {
    auto ret = AmfRtmpPacket::decode(packet);
    if (ret != SUCCESS) {
        return ret;
    }

    return from(amf_object);
}

error_t CommandRtmpPacket::from(AMFObject &amf_object) {
    error_t ret = SUCCESS;
    if (amf_object.o_num < 3) {
        sp_error("amf object num is small %d", amf_object.o_num);
        return ERROR_RTMP_AMF_CMD_CONVERT;
    }

    if ((ret = get_amf_prop(amf_object.o_props, &name)) != SUCCESS) {
        sp_error("amf 1st-prop (%d) is not string, ret: %d", amf_object.o_props->p_type, ret);
        return ret;
    }

    if ((ret = get_amf_prop(amf_object.o_props + 1, &transaction_id)) != SUCCESS) {
        sp_error("amf 2st-prop (%d) is not number ret: %d", amf_object.o_props->p_type, ret);
        return ret;
    }

    if ((ret = get_amf_prop(amf_object.o_props + 2, &object)) != SUCCESS) {
        sp_error("amf 3rd-prop (%d) is not object ret: %d", amf_object.o_props->p_type, ret);
        return ERROR_RTMP_AMF_CMD_CONVERT;
    }

    if ((ret = invoke_from(amf_object)) != SUCCESS) {
        return ret;
    }

    // in case free twice
    if (&amf_object != &this->amf_object) {
        this->amf_object = amf_object;
        memset(&amf_object, 0, sizeof(amf_object));
    }

    return ret;
}

error_t CommandRtmpPacket::invoke_from(AMFObject &amf_object) {
    return SUCCESS;
}

error_t ConnectRtmpPacket::invoke_from(AMFObject &amf_object) {
    error_t ret = SUCCESS;

    if (!equal_val(&name, "connect")) {
        sp_error("amf 1st-prop (%.*s) is not connect", name.av_len, name.av_val);
        return ERROR_RTMP_AMF_CMD_CONVERT;
    }

    if ((ret = get_amf_string(object, "app", &app)) != SUCCESS) {
        sp_error("amf connect has empty app");
        return ret;
    }

    if ((ret = get_amf_string(object, "tcUrl", &tc_url)) != SUCCESS) {
        sp_error("amf connect has empty tcUrl");
        return ret;
    }

    sp_info("connect amf success");
    return ret;
}

error_t CreateStreamRtmpPacket::invoke_from(AMFObject &amf_object) {
    error_t ret = SUCCESS;

    if (!equal_val(&name, "createStream")) {
        sp_error("amf 1st-prop (%.*s) is not createStream", name.av_len, name.av_val);
        return ERROR_RTMP_AMF_CMD_CONVERT;
    }

    return ret;
}

error_t PlayRtmpPacket::invoke_from(AMFObject &amf_object) {
    error_t ret = SUCCESS;

    if (!equal_val(&name, "play")) {
        sp_error("amf 1st-prop (%.*s) is not play", name.av_len, name.av_val);
        return ERROR_RTMP_AMF_CMD_CONVERT;
    }

    if (amf_object.o_num < 4) {
        sp_error("amf 4st-prop (%.*s) is not play", name.av_len, name.av_val);
        return ERROR_RTMP_AMF_CMD_CONVERT;
    }

    AVal stream;
    if ((ret = get_amf_prop(amf_object.o_props + 3, &stream)) != SUCCESS) {
        sp_error("amf 3rd-prop (%d) is not number ret: %d", amf_object.o_props->p_type, ret);
        return ERROR_RTMP_AMF_CMD_CONVERT;
    }

    stream_params = std::string(stream.av_val, stream.av_len);
    return SUCCESS;
}

error_t IRtmpPacket::encode(WrapRtmpPacket& /** packet **/) {
    sp_error("not implements");
    return ERROR_RTMP_NOT_IMPL;
}

bool RtmpPacketDecoder::is_set_chunked_size(int pkt_type) {
    return pkt_type == RTMP_PACKET_TYPE_CHUNK_SIZE;
}

bool RtmpPacketDecoder::is_abort_message(int pkt_type) {
    return pkt_type == RTMP_PACKET_TYPE_ABORT_STREAM;
}

bool RtmpPacketDecoder::is_ack(int pkt_type) {
    return pkt_type == RTMP_PACKET_TYPE_ACK;
}

bool RtmpPacketDecoder::is_user_control(int pkt_type) {
    return pkt_type == RTMP_PACKET_TYPE_USER_CONTROL;
}

bool RtmpPacketDecoder::is_ack_win_size(int pkt_type) {
    return pkt_type == RTMP_PACKET_TYPE_WIN_ACK_SIZE;
}

bool RtmpPacketDecoder::is_set_peer_bandwidth(int pkt_type) {
    return pkt_type == RTMP_PACKET_TYPE_SET_PEER_BANDWIDTH;
}

bool RtmpPacketDecoder::is_command(int pkt_type) {
    return pkt_type == RTMP_PACKET_TYPE_AMF0_DATA ||
            pkt_type == RTMP_PACKET_TYPE_AMF0_CMD ||
            pkt_type == RTMP_PACKET_TYPE_AMF3_DATA ||
            pkt_type == RTMP_PACKET_TYPE_AMF3_CMD;
}

bool RtmpPacketDecoder::is_amf0_command(int pkt_type) {
    return pkt_type == RTMP_PACKET_TYPE_AMF0_CMD;
}

bool RtmpPacketDecoder::is_amf3_command(int pkt_type) {
    return pkt_type == RTMP_PACKET_TYPE_AMF3_CMD;
}

error_t RtmpPacketDecoder::decode(WrapRtmpPacket &pkt, PIRTMPPacket &result) {
    error_t ret  = SUCCESS;
    auto& packet = pkt.packet;
    int pkt_type = packet.m_packetType;

    sp_info("pkt_type: %d", pkt_type);
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
        case RTMP_PACKET_TYPE_AMF3_CMD: {
            sp_info("amf pkt_type: %d", pkt_type);

            auto amf = std::make_unique<AmfRtmpPacket>();
            ret      = amf->decode(pkt);

            if (ret != SUCCESS) {
                sp_error("decode failed ret %d", ret);
                return ret;
            }
            ret = amf->convert_self(result);
            sp_info("final decode ret %d", ret);
        }
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
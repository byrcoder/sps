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
// Created by byrcoder on 2021/7/3.
//

#include <librtmp/sps_librtmp_packet.hpp>

#include <librtmp/amf.h>

#include <sstream>

#include <sps_log.hpp>

namespace sps {


error_t get_amf_prop(AMFObjectProperty *prop, AMFObject **obj) {
    if (prop->p_UTCoffset == AMF_INVALID) {
        return ERROR_RTMP_AMF_PROP_NOT_FOUND;
    }

    if (prop->p_type != AMF_OBJECT && prop->p_type != AMF_NULL) {
        return ERROR_RTMP_AMF_PROP_TYPE;
    }
    *obj = &prop->p_vu.p_object;
    return SUCCESS;
}

error_t get_amf_prop(AMFObjectProperty *prop, AVal *val) {
    if (prop->p_UTCoffset == AMF_INVALID) {
        return ERROR_RTMP_AMF_PROP_NOT_FOUND;
    }

    if (prop->p_type != AMF_STRING) {
        return ERROR_RTMP_AMF_PROP_TYPE;
    }
    *val = prop->p_vu.p_aval;
    return SUCCESS;
}

error_t get_amf_prop(AMFObjectProperty *prop, double *num) {
    if (prop->p_UTCoffset == AMF_INVALID) {
        return ERROR_RTMP_AMF_PROP_NOT_FOUND;
    }

    if (prop->p_type != AMF_NUMBER) {
        return ERROR_RTMP_AMF_PROP_TYPE;
    }
    *num = prop->p_vu.p_number;
    return SUCCESS;
}

error_t get_amf_object(AMFObject *obj, const char *name, AMFObject **obj_val) {
    AVal found = AMF_VAL_STRING(name);

    AMFObjectProperty *prop = AMF_GetProp(obj, &found, -1);
    return get_amf_prop(prop, obj_val);
}

error_t get_amf_string(AMFObject *obj, const char *name, AVal *str_val) {
    AVal found = AMF_VAL_STRING(name);

    AMFObjectProperty *prop = AMF_GetProp(obj, &found, -1);
    return get_amf_prop(prop, str_val);
}

error_t get_amf_num(AMFObject *obj, const char *name, double *num) {
    AVal found = AMF_VAL_STRING(name);

    AMFObjectProperty *prop = AMF_GetProp(obj, &found, -1);
    return get_amf_prop(prop, num);
}

// work as librtmp
char *AMF_EncodeNull(char *start) {
    *start = 0x05;
    return start + 1;
}

char *AMF_EncodeStartObject(char *start) {
    *start = 0x03;
    return start + 1;
}

char *AMF_EncodeEndObject(char *start) {
    *start++ = 0x00;
    *start++ = 0x00;
    *start++ = 0x09;
    return start;
}

bool equal_val(AVal *src, const char *c) {
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

bool WrapRtmpPacket::is_video() const {
    return packet.m_body && packet.m_packetType == RTMP_PACKET_TYPE_VIDEO;
}

bool WrapRtmpPacket::is_audio() const {
    return packet.m_body && packet.m_packetType == RTMP_PACKET_TYPE_AUDIO;
}

bool WrapRtmpPacket::is_script() const {
    return packet.m_body && (packet.m_packetType == RTMP_PACKET_TYPE_AMF0_DATA
                             || packet.m_packetType == RTMP_PACKET_TYPE_AMF3_DATA);
}

uint8_t *WrapRtmpPacket::data() const {
    return (uint8_t *) packet.m_body;
}

size_t WrapRtmpPacket::size() const {
    return packet.m_nBodySize;
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
       << "amf_type: " << amf->p_type << "\t";
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

error_t AmfRtmpPacket::decode(WrapRtmpPacket &pkt) {
    error_t ret = SUCCESS;
    auto &packet = pkt.packet;

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
                 *packet.m_body, *(packet.m_body + 1));
        return ret;
    }

    sp_info ("amf num: %d", amf_object.o_num);
    for (int i = 0; i < amf_object.o_num; ++i) {
        auto amf = amf_object.o_props + i;
        sp_debug("%d. %s", i, debug_amf_object(amf).c_str());
    }
    return SUCCESS;
}

error_t AmfRtmpPacket::convert_self(PIRTMPPacket &result) {
    if (amf_object.o_num <= 0) {
        sp_error("amf not object num %d <= 0", amf_object.o_num);
        return ERROR_RTMP_AMF_CMD_CONVERT;
    }

    AVal name = {nullptr, 0};
    auto ret = get_amf_prop(amf_object.o_props, &name);
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
    } else if (equal_val(&name, "play")) {
        auto conn = std::make_unique<PlayRtmpPacket>();

        if (conn->from(amf_object) != SUCCESS) {
            return ERROR_RTMP_AMF_CMD_CONVERT;
        }

        result = std::move(conn);
    } else if (equal_val(&name, "publish")) {
        auto conn = std::make_unique<PublishRtmpPacket>();

        if (conn->from(amf_object) != SUCCESS) {
            return ERROR_RTMP_AMF_CMD_CONVERT;
        }

        result = std::move(conn);
    } else if (equal_val(&name, "releaseStream")) {
        auto conn = std::make_unique<ReleaseStreamRtmpPacket>();

        if (conn->from(amf_object) != SUCCESS) {
            return ERROR_RTMP_AMF_CMD_CONVERT;
        }

        result = std::move(conn);
    } else if (equal_val(&name, "FCPublish")) {
        auto conn = std::make_unique<FCPublishRtmpPacket>();

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

error_t NetStreamRtmpPacket::invoke_from(AMFObject &amf_object) {
    error_t ret = SUCCESS;

    if (!equal_val(&name, expect_command_name.c_str())) {
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

PlayRtmpPacket::PlayRtmpPacket() {
    this->expect_command_name = "play";
}

PublishRtmpPacket::PublishRtmpPacket() {
    this->expect_command_name = "publish";
}

error_t PublishRtmpPacket::invoke_from(AMFObject &amf_object) {
    error_t ret = NetStreamRtmpPacket::invoke_from(amf_object);

    if (ret != SUCCESS) {
        sp_error("fail publish packet ret: %d", ret);
        return ret;
    }

    if (amf_object.o_num > 5) {
        AVal stream;
        if ((ret = get_amf_prop(amf_object.o_props + 4, &stream)) != SUCCESS) {
            sp_error("amf 3rd-prop (%d) is not number ret: %d", amf_object.o_props->p_type, ret);
            return ERROR_RTMP_AMF_CMD_CONVERT;
        }

        // live, record, append
        publish_type = std::string(stream.av_val, stream.av_len);
    }
    return SUCCESS;
}

FCPublishRtmpPacket::FCPublishRtmpPacket() {
    this->expect_command_name = "FCPublish";
}

ReleaseStreamRtmpPacket::ReleaseStreamRtmpPacket() {
    this->expect_command_name = "releaseStream";
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
    error_t ret = SUCCESS;
    auto &packet = pkt.packet;
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
            auto amf = std::make_unique<AmfRtmpPacket>();
            ret = amf->decode(pkt);

            sp_info("amf pkt_type: %d", pkt_type);
            if (ret != SUCCESS) {
                sp_error("decode failed ret %d", ret);
                return ret;
            }
            ret = amf->convert_self(result);
            sp_info("final decode ret %d", ret);
        }
            break;
        case RTMP_PACKET_TYPE_AUDIOS:
        case RTMP_PACKET_TYPE_VIDEOS:

            break;
        default:
            sp_warn("ignore pkt_type: %d", pkt_type);
            return SUCCESS;
    }

    return ret;
}

}

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

#ifndef SPS_URL_LIBRTMP_PACKET_HPP
#define SPS_URL_LIBRTMP_PACKET_HPP

#include <librtmp/rtmp.h>

#include <string>

#include <sps_error.hpp>
#include <sps_typedef.hpp>

#define RTMP_PACKET_TYPE_SET_CHUNK_SIZE                 RTMP_PACKET_TYPE_CHUNK_SIZE // 0x01
#define RTMP_PACKET_TYPE_ABORT_STREAM                   0x02  // 0x02
#define RTMP_PACKET_TYPE_ACK                            RTMP_PACKET_TYPE_BYTES_READ_REPORT // 0x03
#define RTMP_PACKET_TYPE_USER_CONTROL                   RTMP_PACKET_TYPE_CONTROL  // 0x04
#define RTMP_PACKET_TYPE_WIN_ACK_SIZE                   RTMP_PACKET_TYPE_SERVER_BW  // 0x05
#define RTMP_PACKET_TYPE_SET_PEER_BANDWIDTH             RTMP_PACKET_TYPE_CLIENT_BW  // 0x06

#define RTMP_PACKET_TYPE_AUDIOS                         RTMP_PACKET_TYPE_AUDIO  // 0x08
#define RTMP_PACKET_TYPE_VIDEOS                         RTMP_PACKET_TYPE_VIDEO  // 0x09

// amf3
#define RTMP_PACKET_TYPE_AMF3_DATA           0x0F   // 15
#define RTMP_PACKET_TYPE_AMF3_CMD            0x11   // 17

// amf0
#define RTMP_PACKET_TYPE_AMF0_DATA           0x12  // 18
#define RTMP_PACKET_TYPE_AMF0_CMD            0x14  // 20

// rtmp connect chunked stream id
#define RTMP_PACKET_STREAM_ID_CONTROL           0x02
#define RTMP_PACKET_STREAM_ID_NET_CONNECT       0x03
#define RTMP_PACKET_STREAM_ID_DATA              0x05
#define RTMP_PACKET_STREAM_ID_NET_STREAM        0x08

#define AMF_VAL_STRING(c) { .av_val = (char*) c, .av_len = (int) strlen(c)}
#define AMF_CONST_VAL_STRING(c) { .av_val = (char*) c, .av_len = (int) sizeof(c) -1}
#define AMF_INIT_VAL_STRING(av, c) av = AMF_VAL_STRING(c)

namespace sps {

// prop check valid
error_t get_amf_prop(AMFObjectProperty *prop, AMFObject **obj);

error_t get_amf_prop(AMFObjectProperty *prop, AVal *val);

error_t get_amf_prop(AMFObjectProperty *prop, double *num);

error_t get_amf_object(AMFObject *obj, const char *name, AMFObject **obj_val);

error_t get_amf_string(AMFObject *obj, const char *name, AVal *str_val);

error_t get_amf_num(AMFObject *obj, const char *name, double *num);

char *AMF_EncodeNull(char *start);

char *AMF_EncodeStartObject(char *start);

char *AMF_EncodeEndObject(char *start);

bool equal_val(AVal *src, const char *c);

class WrapRtmpPacket {
 public:
    explicit WrapRtmpPacket(bool own = true);

    ~WrapRtmpPacket();

 public:
    void reset();

 public:
    bool is_video() const;

    bool is_audio() const;

    bool is_script() const;

    uint8_t *data() const;

    size_t size() const;

 public:
    bool own;
    RTMPPacket packet{0};
};

class IRtmpPacket {
 public:
    virtual ~IRtmpPacket() = default;

 public:
    virtual error_t decode(WrapRtmpPacket &packet) = 0;

    error_t encode(WrapRtmpPacket &packet);
};

typedef std::unique_ptr<IRtmpPacket> PIRTMPPacket;

class AmfRtmpPacket : public IRtmpPacket {
 public:
    AmfRtmpPacket();

    ~AmfRtmpPacket() override;

 public:
    error_t decode(WrapRtmpPacket &packet) override;

 public:
    error_t convert_self(PIRTMPPacket &result);

 public:
    AMFObject amf_object{0};
};

class CommandRtmpPacket : public AmfRtmpPacket {
 public:
    error_t decode(WrapRtmpPacket &packet) override;

 public:
    error_t from(AMFObject &amf_object);

    virtual error_t invoke_from(AMFObject &amf_object);

 public:
    AVal name = {nullptr, 0};
    double transaction_id = 0;
    AMFObject *object = nullptr;
};

class ConnectRtmpPacket : public CommandRtmpPacket {
 public:
    error_t invoke_from(AMFObject &amf_object) override;

 public:
    AVal tc_url = {nullptr, 0};
    AVal app = {nullptr, 0};
    AVal flash_ver = {nullptr, 0};
};

class CreateStreamRtmpPacket : public CommandRtmpPacket {
 public:
    error_t invoke_from(AMFObject &amf_object) override;
};

class NetStreamRtmpPacket : public CommandRtmpPacket {
 public:
    error_t invoke_from(AMFObject &amf_object) override;

 public:
    std::string expect_command_name;
    std::string stream_params;
};

class PlayRtmpPacket : public NetStreamRtmpPacket {
 public:
    PlayRtmpPacket();
};

class PublishRtmpPacket : public NetStreamRtmpPacket {
 public:
    PublishRtmpPacket();

 public:
    error_t invoke_from(AMFObject &amf_object) override;

 public:
    std::string publish_type;
};

class FCPublishRtmpPacket : public NetStreamRtmpPacket {
 public:
    FCPublishRtmpPacket();
};

class ReleaseStreamRtmpPacket : public NetStreamRtmpPacket {
 public:
    ReleaseStreamRtmpPacket();
};

class RtmpPacketDecoder {
 public:
    static bool is_set_chunked_size(int pkt_type);

    static bool is_abort_message(int pkt_type);

    static bool is_ack(int pkt_type);

    static bool is_user_control(int pkt_type);

    static bool is_ack_win_size(int pkt_type);

    static bool is_set_peer_bandwidth(int pkt_type);

    static bool is_command(int pkt_type);

    static bool is_amf0_command(int pkt_type);

    static bool is_amf3_command(int pkt_type);

    static error_t decode(WrapRtmpPacket &packet, PIRTMPPacket &result);
};

}  // namespace sps

#endif  // SPS_URL_LIBRTMP_PACKET_HPP

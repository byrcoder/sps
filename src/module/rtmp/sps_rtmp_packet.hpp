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

#ifndef SPS_RTMP_PACKET_HPP
#define SPS_RTMP_PACKET_HPP

#include <sps_rtmp_librtmp.hpp>

namespace sps {

class IRtmpPacket {
 public:
    virtual ~IRtmpPacket() = default;

 public:
    virtual error_t decode(WrapRtmpPacket& packet) = 0;
    error_t encode(WrapRtmpPacket& packet);
};
typedef std::unique_ptr<IRtmpPacket> PIRTMPPacket;

class AmfRtmpPacket : public IRtmpPacket {
 public:
    AmfRtmpPacket();
    ~AmfRtmpPacket() override;

 public:
    error_t decode(WrapRtmpPacket& packet) override;

 public:
    error_t convert_self(PIRTMPPacket& result);

 public:
    AMFObject amf_object {0};
};

class CommandRtmpPacket : public AmfRtmpPacket {
 public:
    error_t decode(WrapRtmpPacket& packet) override;

 public:
    error_t from(AMFObject& amf_object);

    virtual error_t invoke_from(AMFObject& amf_object);

 public:
    AVal        name = {nullptr, 0};
    double      transaction_id = 0;
    AMFObject*  object         = nullptr;
};

class ConnectRtmpPacket : public CommandRtmpPacket {
 public:
    error_t invoke_from(AMFObject& amf_object) override;

 public:
    AVal        tc_url    = {nullptr, 0};
    AVal        app       = {nullptr, 0};
    AVal        flash_ver = {nullptr, 0};
};

class CreateStreamRtmpPacket : public CommandRtmpPacket {
 public:
    error_t invoke_from(AMFObject& amf_object) override;
};

class NetStreamRtmpPacket : public CommandRtmpPacket {
 public:
    error_t invoke_from(AMFObject& amf_object) override;

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
    error_t invoke_from(AMFObject& amf_object) override;

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
    static bool    is_set_chunked_size(int pkt_type);
    static bool    is_abort_message(int pkt_type);
    static bool    is_ack(int pkt_type);
    static bool    is_user_control(int pkt_type);
    static bool    is_ack_win_size(int pkt_type);
    static bool    is_set_peer_bandwidth(int pkt_type);
    static bool    is_command(int pkt_type);
    static bool    is_amf0_command(int pkt_type);
    static bool    is_amf3_command(int pkt_type);

    static error_t decode(WrapRtmpPacket& packet, PIRTMPPacket& result);
};

}

#endif  // SPS_RTMP_PACKET_HPP

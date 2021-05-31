#ifndef SPS_MSG_HPP
#define SPS_MSG_HPP

#include <cache/buffer.hpp>

namespace sps {

enum StreamProtocol {
    STREAM_FLV  = 0,
    STREAM_RTMP = 1,
    STREAM_FILE = 2,

    STREAM_UNKN = 100
};

enum PacketType {
    PACKET_HEAD   = 0,
    PACKET_BODY   = 1,
    PACKET_TAIL   = 2,

    MESSAGE_UNKN   = 100
};

enum FrameType {
    FRAME_V_HEAD,
    FRAME_V_I,
    FRAME_V_P,
    FRAME_V_B,

    FRAME_META,

    FRAME_A_HEAD,
    FRAME_A_BODY,

    FRAME_UNKN   = 100
};

class SpsPacket : public CharBuffer {
 public:
    SpsPacket(StreamProtocol protocol, PacketType packet, FrameType frame, const char* buf, int len);

 public:
    StreamProtocol protocol = STREAM_UNKN;
    PacketType     packet   = MESSAGE_UNKN;
    FrameType      frame    = FRAME_UNKN;
};

}


#endif  // SPS_MSG_HPP

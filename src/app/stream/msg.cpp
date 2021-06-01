#include <app/stream/msg.hpp>

namespace sps {

SpsPacket::SpsPacket(StreamProtocol protocol, PacketType packet,
                     FrameType frame, const char* buf, int len) : CharBuffer(buf, len) {
    this->protocol = protocol;
    this->packet   = packet;
    this->frame    = frame;
}

}

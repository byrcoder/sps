#include <app/stream/sps_stream_msg.hpp>

namespace sps {

SpsPacket::SpsPacket(const IAVInputFormat* fmt, PacketType pkt,
        const char* buf, int len) : CharBuffer(buf, len) {
    this->fmt = fmt;
    this->pkt = pkt;
}

}
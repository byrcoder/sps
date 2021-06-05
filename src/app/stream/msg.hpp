#ifndef SPS_MSG_HPP
#define SPS_MSG_HPP

#include <cache/buffer.hpp>
#include <app/stream/dec.hpp>

namespace sps {

enum PacketType {
    HEADER,
    VIDEO,
    AUDIO,
    METADATA,
};

class SpsPacket : public CharBuffer {
 public:
    SpsPacket(const IAVInputFormat* fmt, PacketType pkt, const char* buf, int len);

 public:
    const IAVInputFormat* fmt;
    PacketType pkt;
};

}


#endif  // SPS_MSG_HPP

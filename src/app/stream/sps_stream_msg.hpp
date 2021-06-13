#ifndef SPS_STREAM_MSG_HPP
#define SPS_STREAM_MSG_HPP

#include <cache/sps_cache_buffer.hpp>
#include <app/stream/sps_stream_dec.hpp>

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


#endif  // SPS_STREAM_MSG_HPP

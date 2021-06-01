#ifndef SPS_PROTOCOL_PROTOCOL_HPP
#define SPS_PROTOCOL_PROTOCOL_HPP

#include <cache/buffer.hpp>
#include <typedef.hpp>

#define PROTOCOL_EOF 2000

namespace sps {

class IProtocol {
 public:
    virtual ~IProtocol() = default;

 public:
    virtual error_t read_header(PIBuffer& buffer)  = 0;
    virtual error_t read_message(PIBuffer& buffer) = 0;
    virtual error_t read_tail(PIBuffer& buffer)    = 0;
};

typedef std::shared_ptr<IProtocol> PIProtocol;

}

#endif  // SPS_PROTOCOL_PROTOCOL_HPP

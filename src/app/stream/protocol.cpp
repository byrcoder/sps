#include <app/stream/protocol.hpp>

#include <app/stream/flv.hpp>

namespace sps {

PIProtocol ProtocolFactory::create_protocol(Protocol p, PIReaderWriter io) {
    if (p == Protocol::PFLV) {
        return std::make_shared<FlvProtocol>(io);
    }
    return nullptr;
}

}
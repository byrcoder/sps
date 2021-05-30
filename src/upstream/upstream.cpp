#include <upstream/upstream.hpp>

namespace sps {

IUpstream::IUpstream(PICacheStream cs) {
    this->cs = cs;
}

IUpstream::~IUpstream() {
    cs->close();
}

PICacheStream IUpstream::get_cs() {
    return cs;
}

error_t IUpstream::handler() {
    return up();
}

}

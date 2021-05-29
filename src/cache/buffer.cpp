#include <cache/buffer.hpp>

#include <string.h>

namespace sps {

PCharBuffer CharBuffer::copy(char *buf, int len) {
    return std::make_shared<CharBuffer>(buf, len);
}

CharBuffer::CharBuffer(char *buf, int len) {
    this->buf = new char[len];
    this->len = len;
    memcpy(this->buf, buf, len);
}

CharBuffer::~CharBuffer() noexcept {
    delete []buf;
}

}

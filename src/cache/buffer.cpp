#include <cache/buffer.hpp>

#include <string.h>

namespace sps {

PCharBuffer CharBuffer::copy(const char *buf, int len) {
    return std::make_shared<CharBuffer>(buf, len);
}

CharBuffer::CharBuffer(const char *buf, int len) {
    this->buf = new char[len];
    this->len = len;
    memcpy(this->buf, buf, len);
}

CharBuffer::~CharBuffer() {
    delete []buf;
}

}

/*****************************************************************************
MIT License
Copyright (c) 2021 byrcoder

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*****************************************************************************/

#include <sps_cache_buffer.hpp>

#include <cstring>

namespace sps {

PCharBuffer CharBuffer::deep_copy(uint8_t *buf, uint32_t len,
                                  uint32_t head_len) {
    return std::make_shared<CharBuffer>(buf, len, head_len);
}

PCharBuffer CharBuffer::create_buf(uint32_t cap) {
    return std::make_shared<CharBuffer>(cap);
}

CharBuffer::CharBuffer(uint8_t *buf, uint32_t len, uint32_t head_len) {
    this->buf = new uint8_t[len + head_len];
    this->len = len;
    this->head_len = head_len;
    memcpy(this->buf + head_len, buf, len);
    cap_len = len;
}

CharBuffer::CharBuffer(uint32_t cap, uint32_t head_len) {
    buf      = new uint8_t[cap + head_len];
    len      = 0;
    cap_len  = cap;
    this->head_len = head_len;
}

CharBuffer::~CharBuffer() {
    delete []buf;
}

}  // namespace sps

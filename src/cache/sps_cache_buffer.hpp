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

#ifndef SPS_CACHE_BUFFER_HPP
#define SPS_CACHE_BUFFER_HPP

#include <cassert>
#include <cstring>

#include <memory>

namespace sps {

class IBuffer {
 public:
    virtual ~IBuffer() = default;

 public:
    virtual uint8_t *buffer() = 0;

    virtual uint32_t size() = 0;

    virtual uint32_t cap() {
        return size();
    }
};

typedef std::shared_ptr<IBuffer> PIBuffer;

class CharBuffer;
typedef std::shared_ptr<CharBuffer> PCharBuffer;

class CharBuffer : public IBuffer {
 public:
    static PCharBuffer deep_copy(uint8_t* buf, uint32_t len,
                                 uint32_t head_len = 0);

    static PCharBuffer create_buf(uint32_t cap);

 public:
    CharBuffer(uint8_t* buf, uint32_t len, uint32_t head_len = 0);
    CharBuffer(uint32_t cap);
    ~CharBuffer() override;

 public:
    uint8_t*   buffer() override      { return buf+head_len; }
    uint32_t   size() override        { return len; }
    uint32_t   cap() override         { return cap_len; }

 public:
    void  append(uint8_t* p, uint32_t sz) {
        assert(remain() >= sz);
        memcpy(pos(), p, sz);
        len += sz;
    }
    uint8_t*   pos()              { return buf + len; }
    uint32_t   remain()           { return cap_len - len; }
    uint8_t*   head_pos()         { return head_len ? buf : nullptr; }
    uint32_t   head_size()            { return head_len; }
 private:
    uint8_t*      buf      = nullptr;
    uint32_t      len      = 0;
    uint32_t      head_len = 0;
    uint32_t      cap_len  = 0;
};

}  // namespace sps

#endif  // SPS_CACHE_BUFFER_HPP

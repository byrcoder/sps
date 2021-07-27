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

//
// Created by byrcoder on 2021/7/27.
//

#include <sps_io_bits.hpp>
#include <sps_log.hpp>

namespace sps {

BitContext::BitContext(uint8_t *p, size_t sz) {
    init(p, sz);
}

void BitContext::init(uint8_t *p, size_t sz) {
    buf         =  p;
    buf_pos     = -1;
    this->sz    = sz;
    remain_bits = 0;
}

uint8_t* BitContext::pos() {
    if (remain_bits > 0) {
        return buf + buf_pos;
    }
    return buf + buf_pos + 1;
}

size_t BitContext::size_bits() {
    return (sz - buf_pos - 1) * 8 + remain_bits;
}

size_t BitContext::size() {
    return size_bits() / 8;
}

error_t BitContext::acquire(uint32_t nbyte) {
    return acquire_bits(nbyte * 8);
}

error_t BitContext::acquire_bits(size_t nbit) {
    return size_bits() >= nbit ? SUCCESS : ERROR_IO_NOT_ENOUGH;
}

void BitContext::read_bytes(uint8_t* c, size_t n) {
    assert(remain_bits == 0 || remain_bits == 8);
    if (remain_bits == 8) {
        memcpy((char *) c, buf + buf_pos, n);
    } else {
        memcpy((char *) c, buf + buf_pos+1, n);
    }
    buf_pos += n;
}

void BitContext::skip_bytes(size_t n) {
    buf_pos += n;
}

uint8_t BitContext::read_int8() {
    return read_bits(8);
}

uint16_t BitContext::read_int16() {
    return read_bits(16);
}

uint32_t BitContext::read_int24() {
    return read_bits(24);
}

uint32_t BitContext::read_int32() {
    return read_bits(32);
}

uint32_t BitContext::read_bits(size_t n) {
    uint32_t num = 0;

    while (n > 0) {
        if (remain_bits == 0) {
            next();
        }

        uint32_t nr = std::min((uint32_t)n, (uint32_t)remain_bits);
        num <<= nr;

        sp_debug("before n %lu, remain %lu, nr %d, remain_value %x, num %x",
                n, remain_bits, nr, remain_value, num);

        switch (nr) {
            case 1:
                num |= ((remain_value & 0x80) >> 7);
                break;
            case 2:
                num |= ((remain_value & 0xc0) >> 6);
                break;
            case 3:
                num |= ((remain_value & 0xe0) >> 5);
                break;
            case 4:
                num |= ((remain_value & 0xf0) >> 4);
                break;
            case 5:
                num |= ((remain_value & 0xf8) >> 3);
                break;
            case 6:
                num |= ((remain_value & 0xfc) >> 2);
                break;
            case 7:
                num |= ((remain_value & 0xfe) >> 1);
                break;
            case 8:
                num |= (remain_value & 0xff);
                break;
            default:
                assert(false);
        }

        remain_bits -= nr;
        remain_value <<= nr;  // stay high bit
        n -= nr;

        sp_debug("after n %lu, remain %lu, nr %d, remain_value %x, num %x",
                n, remain_bits, nr, remain_value, num);
    }
    return num;
}

void BitContext::next() {
    assert(remain_bits == 0);
    ++buf_pos;
    remain_bits  = 8;
    remain_value = *(buf+buf_pos);
}

}
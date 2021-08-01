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
    assert(remain_bits == 0);
    memcpy((char *) c, buf + buf_pos + 1, n);
    buf_pos += n;
}

void BitContext::skip_read_bytes(size_t n) {
    buf_pos += n;

    // remain_value is invalid
    if (remain_bits == 0) {
        return;
    }

    remain_value = buf[buf_pos];
    remain_value <<= (8-remain_bits);
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

void BitContext::write_bytes(uint8_t* c, size_t n) {
    assert(remain_bits == 0);
    memcpy((char *) c, buf + buf_pos + 1, n);
    buf_pos += n;
}

void BitContext::skip_write_bytes(size_t n) {
    assert(remain_bits == 0);
    buf_pos += n;
}

void BitContext::write_bits(uint64_t value, size_t n) {
    while (n > 0) {
        if (remain_bits == 0) {
            next_write();
        }

        size_t nr = std::min(remain_bits, n);
        uint32_t nr_value = 0;

        static uint32_t masks[]     =  {
                0x01, 0x03,
                0x07, 0x0f,
                0xf8, 0xfc,
                0xfe, 0xff
        };

        // eg. read value nr = 3, n = 6  value = (101) xxxxxxxx
        nr_value = masks[nr-1] & (value >> (n-nr));

        // eg. write value remain_bit = 7, nr = 3, value = 1 (101) xxxx
        buf[buf_pos] |= (nr_value << (remain_bits - nr));

        n -= nr;
        remain_bits -= nr;
    }
}

void BitContext::write_int8(uint32_t value) {
    write_bits(value, 8);
}

void BitContext::write_int16(uint32_t value) {
    write_bits(value, 16);
}

void BitContext::write_int24(uint32_t value) {
    write_bits(value, 24);
}

void BitContext::write_int32(uint32_t value) {
    write_bits(value, 32);
}

void BitContext::next() {
    assert(remain_bits == 0);
    ++buf_pos;
    remain_bits  = 8;
    remain_value = *(buf+buf_pos);
}

void BitContext::next_write() {
    assert(remain_bits == 0);
    ++buf_pos;
    remain_bits  = 8;
    buf[buf_pos] = 0;  // clear value
}

}
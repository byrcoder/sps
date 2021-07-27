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

#ifndef SPS_IO_BITS_HPP
#define SPS_IO_BITS_HPP

#include <sps_typedef.hpp>

namespace sps {

class BitContext {
 public:
    BitContext(uint8_t* p = nullptr, size_t sz = 0);

 public:
    void init(uint8_t* p, size_t sz);
    uint8_t* pos();

 public:
    size_t size_bits();
    size_t size();  // bytes

 public:
    error_t acquire(uint32_t nbyte);
    error_t acquire_bits(size_t nbit);

 public:
    void     read_bytes(uint8_t* c, size_t n);
    void     skip_bytes(size_t n);

    uint8_t  read_int8();
    uint16_t read_int16();
    uint32_t read_int24();
    uint32_t read_int32();

 public:
    uint32_t read_bits(size_t n);

 private:
    void next();

 private:
    // byte pos info
    uint8_t* buf;
    size_t   sz;
    int32_t  buf_pos;  // current pos

    // last byte not read bit
    size_t   remain_bits;
    // remain bits high bit
    // eg. remain_bits = 3,
    // remain_value = 101 (00000) 101 is not read, 00000 has read
    uint8_t  remain_value;
};

}  // namespace sps

#endif  // SPS_IO_BITS_HPP

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
// Created by byrcoder on 2021/6/18.
//
#ifndef SPS_IO_BYTES_HPP
#define SPS_IO_BYTES_HPP

#include <sps_io.hpp>
#include <memory>

namespace sps {

/**
 *
 *  |(buf)--------(buf_ptr)----------|(buf_end)xxxxxxxx|buf_cap
 *
 *  read:
 *           [buf, buf_ptr) has read data,  used();
 *           [buf_ptr, buf_end) has recv data buf waiting for read (cache buffer) size();
 *           [buf_end, buf_cap) remain buffer for not yet recv data  remain_size();
 *
 *  |(buf)--------(buf_ptr)xxxxxxxxxxxxxxxxxx|buf_cap
 *
 *
 */
class AVBuffer {
 public:
    AVBuffer(size_t cap, bool rewindable);
    AVBuffer(uint8_t* buf, size_t cap, bool rewindable);
    AVBuffer(uint8_t* buf, size_t end, size_t cap, bool rewindable);
    ~AVBuffer();

 public:
    uint8_t*   buffer();
    uint8_t*   pos();
    uint8_t*   end();

    size_t     size() const;
    size_t     no_rewind_cap_size() const;
    size_t     cap_size() const;
    size_t     remain();

    void       skip(size_t n);
    void       rewind();

    void       append(size_t n);

    void       clear();

 public:
    uint8_t*  buf;
    size_t    buf_cap;
    size_t    buf_ptr;
    size_t    buf_end;
    bool      rewindable;

 private:
    bool      own;
};

typedef std::unique_ptr<AVBuffer> PAVBuffer;

class SpsBytesReader;
typedef std::unique_ptr<SpsBytesReader> PSpsBytesReader;
class SpsBytesReader {
 public:
    static PSpsBytesReader create_reader(uint8_t* buf, int len);

 public:
    SpsBytesReader(PIReader& io, PAVBuffer& buf);
    ~SpsBytesReader();

 public:
    size_t size() {
        return buf->size();
    }

    uint8_t* pos() {
        return buf->pos();
    }

 public:
    // call acquire then call following functions
    void     read_bytes(uint8_t* c, size_t n);
    void     skip_bytes(size_t n);
    inline uint8_t  read_int8() {
        uint8_t n = *buf->pos();
        buf->skip(1);
        return n;
    }
    uint16_t read_int16();
    uint32_t read_int24();
    uint32_t read_int32();

    error_t  acquire(uint32_t n);

 private:
    PIReader& io;

 public:
    PAVBuffer& buf;
};

class SpsBytesWriter {
 public:
    explicit SpsBytesWriter(PAVBuffer& buf);

    error_t acquire(uint32_t n);

    inline void write_int8(uint8_t n) {
        *buf->end() = n;
        buf->append(1);
    }

    void    write_int16(uint16_t n);
    void    write_int24(uint32_t n);
    void    write_int32(uint32_t n);
    void    write_bytes(uint8_t* data, size_t n);
    void    skip(size_t n);


 private:
    PAVBuffer& buf;
};

}  // namespace sps

#endif  // SPS_IO_BYTES_HPP

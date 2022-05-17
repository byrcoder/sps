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
#include <sps_io_bytes.hpp>
#include <log/sps_log.hpp>

namespace sps {

AVBuffer::AVBuffer(size_t cap, bool rw) {
    buf = new uint8_t[cap];
    buf_ptr = buf_end = 0;
    buf_cap = cap;
    rewindable = rw;
    own        = true;
}

AVBuffer::AVBuffer(uint8_t *buf, size_t cap, bool rw) {
    this->buf = buf;
    buf_ptr = buf_end = 0;
    this->buf_cap = cap;
    rewindable    = rw;
    own           = false;
}

AVBuffer::AVBuffer(uint8_t* buf, size_t end, size_t cap, bool rw) {
    this->buf = buf;
    buf_ptr = 0;
    this->buf_end = end;
    this->buf_cap = cap;
    rewindable    = rw;
    own           = false;
}

AVBuffer::~AVBuffer() {
    if (own) {
        delete[] buf;
    }
}

uint8_t* AVBuffer::buffer() {
    return buf;
}

uint8_t* AVBuffer::pos() {
    return buf + buf_ptr;
}

uint8_t* AVBuffer::end() {
    return buf + buf_end;
}

size_t AVBuffer::size() const {
    return buf_end - buf_ptr;
}

size_t AVBuffer::no_rewind_cap_size() const {
    return  buf_cap - buf_ptr;  // size() + buf_cap - buf_end;
}

size_t AVBuffer::cap_size() const {
    if (!rewindable) {
        return no_rewind_cap_size();
    } else {
        return buf_cap;
    }
}

size_t AVBuffer::remain() {
    return buf_cap - buf_end;
}

void AVBuffer::skip(size_t n) {
    buf_ptr += n;
}

void AVBuffer::rewind() {
    memmove(buf, buf+buf_ptr, buf_end-buf_ptr);
    buf_end = buf_end - buf_ptr;
    buf_ptr = 0;
}

void AVBuffer::append(size_t n) {
    buf_end += n;
}

void AVBuffer::clear() {
    buf_ptr = buf_end = 0;
}

PBytesReader BytesReader::create_reader(uint8_t* buf, int len) {
    static PIReader null_reader;

    auto av_buffer = std::make_shared<AVBuffer>(
            buf, len, len, false);
    return std::make_unique<BytesReader>(null_reader, av_buffer);
}

BytesReader::BytesReader(PIReader io, PAVBuffer buf):
    io(std::move(io)), buf(std::move(buf)) {
}

BytesReader::~BytesReader() {
}

void BytesReader::read_bytes(uint8_t* c, size_t n) {
    memcpy(c, buf->pos(), n);
    buf->skip(n);
}

size_t BytesReader::size() {
    return buf->size();
}

uint8_t* BytesReader::pos() {
    return buf->pos();
}

void BytesReader::skip_bytes(size_t n) {
    buf->skip(n);
}

uint16_t BytesReader::read_int16() {
    return ((uint32_t) read_int8()) << 8u | read_int8();
}

uint32_t BytesReader::read_int24() {
    return ((uint32_t) read_int8()) << 16u | read_int16();
}

uint32_t BytesReader::read_int32() {
    return ((uint32_t) read_int16()) << 16u |  read_int16();
}

void BytesReader::read_reverse_bytes(uint8_t* c, size_t n) {
    for (int i = 0; i < n; ++i) {
        *c = read_int8();
        --c;
    }
}

error_t BytesReader::acquire(uint32_t n) {
    error_t ret   = SUCCESS;
    size_t  nread = 0;

    if (buf->size() >= n) {
        return ret;
    }

    if (buf->cap_size() < n) {
        sp_error("fail buffer overflow %zu < %u", buf->cap_size(), n);
        return ERROR_IO_BUFFER_FULL;
    }

    if (buf->no_rewind_cap_size() < n) {
        buf->rewind();
    }

    assert(buf->no_rewind_cap_size() >= n);

    do {
        if (!io) {
            sp_error("fail error io eof");
            return ERROR_IO_EOF;
        }

        if ((ret = io->read(buf->end(),
                buf->remain(), nread)) != SUCCESS) {
            sp_error("fail increase buffer read ret: %d", ret);
            return ret;
        }

        buf->buf_end += nread;
    } while (buf->size() < n);

    sp_debug("%s, size:%zu, n:%d", (char*) buf->pos(), buf->size(), n);
    return ret;
}

PBytesWriter BytesWriter::create_writer(uint8_t *buf, int len) {
    return std::make_shared<BytesWriter>(std::make_shared<AVBuffer>(buf, len, false));
}

BytesWriter::BytesWriter(PAVBuffer b): buf(std::move(b)) {
}

error_t BytesWriter::acquire(uint32_t n) {
    return buf->remain() >= n ? SUCCESS : ERROR_IO_BUFFER_FULL;
}

void BytesWriter::write_int16(uint16_t n) {
    write_int8(n >> 8);
    write_int8(n & 0XFF);
}

void BytesWriter::write_int24(uint32_t n) {
    write_int8(n >> 16);
    write_int16(n & 0XFFFF);
}

void BytesWriter::write_int32(uint32_t n) {
    write_int8(n >> 24);
    write_int24(n & 0XFFFFFF);
}

void BytesWriter::write_bytes(uint8_t* data, size_t n) {
    memcpy(buf->end(), data, n);
    buf->append(n);
}

void BytesWriter::skip(size_t n) {
    buf->append(n);
}

}  // namespace sps

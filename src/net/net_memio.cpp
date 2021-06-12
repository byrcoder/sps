#include <net/net_memio.hpp>

#include <string.h>
#include <algorithm>
#include <string>
#include <log/log_logger.hpp>

namespace sps {

MemoryReaderWriter::MemoryReaderWriter(char* buf, int len) {
    init(buf, len);
    line_num = 0;
}

void MemoryReaderWriter::init(char *buf, int len) {
    this->buf = buf;
    this->len = len;
    this->pos = 0;
}

utime_t MemoryReaderWriter::get_recv_timeout() {
    return 0;
}

void MemoryReaderWriter::set_recv_timeout(utime_t ) {

}

bool MemoryReaderWriter::seekable() {
    return true;
}

error_t MemoryReaderWriter::read(void *buf, size_t size, size_t &nread) {
    nread = std::min ((int) size, (int) (len - pos));
    if (nread <= 0) {
        return ERROR_IO_EOF;
    }

    memcpy((char*) buf, this->buf + pos, nread);

    pos += nread;
    return SUCCESS;
}

error_t MemoryReaderWriter::read_line(std::string &line) {
    if (pos >= len) {
        return ERROR_IO_EOF;
    }

    uint32_t start = pos;

    while (pos < len && buf[pos++] != '\n') {
    }

    int tlen = (pos < len) ? (pos-start-1) : (pos-start);

    line = std::string(buf + start, tlen);

    ++line_num;

    return SUCCESS;
}

int MemoryReaderWriter::cur_line_num() {
    return line_num;
}

error_t MemoryReaderWriter::read_fully(void *buf, size_t size, ssize_t *nread) {
    auto n = std::min ((int) size, (int) (len - pos));
    if (n < size || n == 0) {
        return ERROR_IO_EOF;
    }

    sp_info ("%s", buf);
    memcpy((char*) buf, this->buf + pos, n);

    pos += n;

    if (nread) *nread = n;
    return SUCCESS;
}

void MemoryReaderWriter::set_send_timeout(utime_t ) {

}

utime_t MemoryReaderWriter::get_send_timeout() {
    return 0;
}

error_t MemoryReaderWriter::write(void *buf, size_t size) {
    auto left = len - pos;
    if (left < size) {
        return ERROR_MEM_SOCKET_WRITE;
    }

    memcpy(this->buf + pos, buf, size);
    return SUCCESS;
}

}
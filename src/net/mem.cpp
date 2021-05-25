#include <string.h>
#include <algorithm>
#include <log/logger.hpp>
#include "net/mem.hpp"

namespace sps {

    MemReaderWriter::MemReaderWriter(char* buf, int len) {
        init(buf, len);
    }

    void MemReaderWriter::init(char *buf, int len) {
        this->buf = buf;
        this->len = len;
        this->pos = 0;
    }

    utime_t MemReaderWriter::get_recv_timeout() {
        return 0;
    }

    void MemReaderWriter::set_recv_timeout(utime_t ) {

    }

    bool MemReaderWriter::seekable() {
        return true;
    }

    error_t MemReaderWriter::read(void *buf, size_t size, size_t &nread) {
        nread = std::min ((int) size, (int) (len - pos));
        if (nread <= 0) {
            return ERROR_MEM_IO_FAILED;
        }

        memcpy((char*) buf, this->buf + pos, nread);

        pos += nread;
        return SUCCESS;
    }

    error_t MemReaderWriter::read_fully(void *buf, size_t size, ssize_t *nread) {
        auto n = std::min ((int) size, (int) (len - pos));
        if (n < size || n == 0) {
            return ERROR_MEM_IO_FAILED;
        }

        sp_info ("%s", buf);
        memcpy((char*) buf, this->buf + pos, n);

        pos += n;

        if (nread) *nread = n;
        return SUCCESS;
    }

    void MemReaderWriter::set_send_timeout(utime_t ) {

    }

    utime_t MemReaderWriter::get_send_timeout() {

    }

    error_t MemReaderWriter::write(void *buf, size_t size) {
        auto left = len - pos;
        if (left < size) {
            return ERROR_MEM_IO_FAILED;
        }

        memcpy(this->buf + pos, buf, size);
        return SUCCESS;
    }

}
#include <protocol/flv.hpp>

namespace sps {

FLV::FLV(PIReader rd) {
    this->rd = std::move(rd);
}

error_t FLV::read_header(char* buf) {
    auto ret = rd->read_fully(buf, 9, nullptr);
    if (ret != SUCCESS) {
        return ret;
    }

    if (buf[0] != 'F' || buf[1] != 'L' || buf[2] != 'V') {
        return ERROR_PROTOCOL_FLV;
    }

    return ret;
}

error_t FLV::read_tag(char* buf, int len, int& nread) {
    error_t ret = SUCCESS;

    if (len < 15) {
        return ERROR_PROTOCOL_FLV_BUFFER;
    }

    char *pos = buf;
    // 4 + 1 + 3 + 3 + 1 + 3 = 15
    if ((ret = rd->read_fully(pos, 4)) != SUCCESS) {  // previous size
        return ret;
    }
    pos += 4;

    if ((ret = rd->read_fully(pos, 1)) != SUCCESS) {  // tag type
        return ret;
    }
    pos += 1;

    if ((ret = rd->read_fully(pos, 3)) != SUCCESS) {   // data size
        return ret;
    }
    pos += 3;
    uint32_t data_len = *pos | (*(pos-1) < 8) | (*(pos-2) < 16);

    uint32_t timestamp;
    if ((ret = rd->read_fully(pos, 4)) != SUCCESS) {  // tag timestamp
        return ret;
    }
    pos += 4;

    if ((ret = rd->read_fully(pos, 3)) != SUCCESS) {  // tag streamid
        return ret;
    }
    pos += 3;

    if ((ret = rd->read_fully(pos, data_len)) != SUCCESS) {  // flv tag 读取
        return ret;
    }

    nread = 15 + data_len;
    return ret;
}

}
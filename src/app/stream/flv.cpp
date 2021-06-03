#include <app/stream/flv.hpp>
#include <app/stream/msg.hpp>
namespace sps {

Flv::Flv(PIReader rd) {
    this->rd = std::move(rd);
}

error_t Flv::read_header(char* buf) {
    auto ret = rd->read_fully(buf, 9, nullptr);
    if (ret != SUCCESS) {
        return ret;
    }

    if (buf[0] != 'F' || buf[1] != 'L' || buf[2] != 'V') {
        return ERROR_PROTOCOL_FLV;
    }

    return ret;
}

error_t Flv::read_tag(char* buf, int len, int& nread) {
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

    if (data_len + 15 > len) {
        return ERROR_MEM_OVERFLOW;
    }

    if ((ret = rd->read_fully(pos, data_len)) != SUCCESS) {  // flv tag 读取
        return ret;
    }

    nread = 15 + data_len;
    return ret;
}

FlvProtocol::FlvProtocol(PIReader rd) : buf(max_len, 0)  {
    flv = std::make_shared<Flv>(rd);
}

error_t FlvProtocol::read_header(PIBuffer &buffer) {
    auto ret = flv->read_header(&buf[0]);
    if (ret != SUCCESS) return ret;
    buffer = std::make_shared<SpsPacket>(StreamProtocol::STREAM_FLV,
            PacketType::PACKET_HEAD, FrameType::FRAME_UNKN, &buf[0], 9);
    return ret;
}

error_t FlvProtocol::read_message(PIBuffer& buffer) {
    int nread = 0;
    auto ret = flv->read_tag(&buf[0], max_len, nread);
    if (ret != SUCCESS) return ret;
    buffer = std::make_shared<SpsPacket>(StreamProtocol::STREAM_FLV,
                                         PacketType::PACKET_BODY, FrameType::FRAME_UNKN, &buf[0], nread);
    return ret;
}

error_t FlvProtocol::read_tail(PIBuffer& buffer) {
    return SUCCESS;
}

}
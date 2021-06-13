#include <app/stream/sps_stream_flvdec.hpp>
#include <app/stream/sps_stream_msg.hpp>
#include <app/url/sps_url.hpp>

#include <net/sps_net_socket.hpp>

namespace sps {

FlvDecoder::FlvDecoder(PIReader rd) : buf(max_len, 0)  {
    this->rd = std::move(rd);
}

error_t FlvDecoder::read_header(PIBuffer &buffer) {
    auto ret = rd->read_fully(&buf[0], 9, nullptr);
    if (ret != SUCCESS) {
        return ret;
    }

    if (buf[0] != 'F' || buf[1] != 'L' || buf[2] != 'V') {
        return ERROR_FLV_PROBE;
    }

    buffer = std::make_shared<SpsPacket>(fmt,
            PacketType::HEADER, &buf[0], 9);
    return ret;
}

error_t FlvDecoder::read_message(PIBuffer& buffer) {
    const int flv_head_len = 15;

    error_t ret = SUCCESS;
    int nread = 0;
    int len = max_len;

    if (len < flv_head_len) {
        return ERROR_FLV_BUFFER_OVERFLOW;
    }

    char *pos = &buf[0];
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

    nread = flv_head_len + data_len;

    buffer = std::make_shared<SpsPacket>(fmt,
                                         PacketType::HEADER,
                                         &buf[0], nread);
    return ret;
}

error_t FlvDecoder::read_tail(PIBuffer& buffer) {
    return SUCCESS;
}

error_t FlvDecoder::probe(PIBuffer &buffer) {
    auto buf = buffer->buffer();
    int  len = buffer->size();

    if (len < 3 || buf[0] != 'F' || buf[1] != 'L' || buf[2] != 'V') {
        return ERROR_FLV_PROBE;
    }
    return SUCCESS;
}

FlvInputFormat::FlvInputFormat() :
    IAVInputFormat("flv", "flv") {
}

PIAvDemuxer FlvInputFormat::create(PIURLProtocol p) {
    return std::make_shared<FlvDecoder>(p);
}

}
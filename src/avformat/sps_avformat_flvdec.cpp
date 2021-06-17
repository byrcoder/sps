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

#include <sps_avformat_flvdec.hpp>
#include <sps_avformat_packet.hpp>
#include <sps_url.hpp>
#include <sps_io_socket.hpp>
#include "sps_avformat_flv.hpp"

namespace sps {

FlvDemuxer::FlvDemuxer(PIReader rd) : buf(max_len, 0)  {
    this->rd = std::move(rd);
}

error_t FlvDemuxer::read_header(PSpsAVPacket &buffer) {
    auto ret = rd->read_fully(&buf[0], 9, nullptr);
    if (ret != SUCCESS) {
        return ret;
    }

    if (buf[0] != 'F' || buf[1] != 'L' || buf[2] != 'V') {
        return ERROR_FLV_PROBE;
    }

    // version buf[3]

    int flags = ((int) buf[4]) & (FLV_HEADER_FLAG_HASVIDEO | FLV_HEADER_FLAG_HASAUDIO); // flags
    // header_len buf[5...8]

    buffer = SpsAVPacket::create(SpsAVPacketType::AV_PKT_TYPE_HEADER,
                                AV_STREAM_TYPE_NB,
                                &buf[0], 9,
                                0, 0, flags, 0);
    buffer->flags = buf[4];

    return ret;
}

error_t FlvDemuxer::read_message(PSpsAVPacket& buffer) {
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

//    buffer = std::make_shared<SpsAVPacket>(fmt,
//                                           PacketType::HEADER,
//                                           &buf[0], nread);

    buffer = SpsAVPacket::create(SpsAVPacketType::AV_PKT_DATA,
                                 AV_STREAM_TYPE_NB,
                                 &buf[0], nread,
                                 0, 0);

    return ret;
}

error_t FlvDemuxer::read_tail(PSpsAVPacket& buffer) {
    return SUCCESS;
}

error_t FlvDemuxer::probe(PSpsAVPacket& buffer) {
    auto buf = buffer->buffer();
    int  len = buffer->size();

    if (len < 3 || buf[0] != 'F' || buf[1] != 'L' || buf[2] != 'V') {
        return ERROR_FLV_PROBE;
    }
    return SUCCESS;
}

FlvAVInputFormat::FlvAVInputFormat() :
    IAVInputFormat("flv", "flv") {
}

PIAVDemuxer FlvAVInputFormat::_create(PIReader p) {
    return std::make_shared<FlvDemuxer>(p);
}

}
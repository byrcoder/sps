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

#include <sps_avformat_flv.hpp>
#include <sps_avformat_flvdec.hpp>
#include <sps_avformat_packet.hpp>
#include <sps_io_bytes.hpp>

namespace sps {

static const int max_len = 1 * 1024 * 1024;

FlvDemuxer::FlvDemuxer(PIReader rd) {
    buf       = std::make_unique<AVBuffer>(max_len, true);
    this->_rd = rd;
    this->rd  = std::make_unique<SpsBytesReader>(_rd, buf);
}

error_t FlvDemuxer::read_header(PSpsAVPacket &buffer) {
    auto ret = rd->acquire(9);

    if (ret != SUCCESS) {
        return ret;
    }
    uint8_t* buf = this->buf->pos();

    if (buf[0] != 'F' || buf[1] != 'L' || buf[2] != 'V') {
        return ERROR_FLV_PROBE;
    }

    // version buf[3] skip

    int flags = ((int) buf[4]) & (FLV_HEADER_FLAG_HASVIDEO | FLV_HEADER_FLAG_HASAUDIO); // flags
    // header_len buf[5...8]

    buffer = SpsAVPacket::create(SpsAVPacketType::AV_PKT_TYPE_HEADER,
                                AV_STREAM_TYPE_NB,
                                buf, 9,
                                0, 0, flags, 0);
    buffer->flags = buf[4];

    rd->skip_bytes(9);

    return ret;
}

error_t FlvDemuxer::read_message(PSpsAVPacket& buffer) {
    const int flv_head_len = 15;

    error_t  ret    = SUCCESS;
    int      nread  = 0;
    uint32_t previous_size = 0;
    uint32_t pos           = 0;
    uint32_t tag_type      = 0;
    uint32_t data_size     = 0;
    uint32_t timestamp     = 0;
    uint32_t stream_id     = 0;

    if ((ret = rd->acquire(flv_head_len)) != SUCCESS) {
        return ERROR_FLV_BUFFER_OVERFLOW;
    }
    previous_size = rd->read_int32(); // previous
    tag_type      = rd->read_int8();  // tag_type
    data_size     = rd->read_int24(); // data_size
    timestamp     = rd->read_int24(); // timestamp low 24 bit
    timestamp    |= ((uint32_t) rd->read_int8() << 24u); // high 8 bit
    stream_id     = rd->read_int24(); // streamid

    if ((ret = rd->acquire(data_size)) != SUCCESS) {
        return ERROR_MEM_OVERFLOW;
    }

    // TODO: FIXME
    buffer = SpsAVPacket::create(SpsAVPacketType::AV_PKT_DATA,
                                 AV_STREAM_TYPE_NB,
                                 buf->pos(), data_size,
                                 0, 0);

    rd->skip_bytes(data_size);
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
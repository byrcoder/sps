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
// Created by byrcoder on 2021/6/16.
//

#include <sps_avformat_flvenc.hpp>
#include <sps_io_bytes.hpp>
#include <log/sps_log.hpp>

namespace sps{

FlvAVMuxer::FlvAVMuxer(PIWriter writer) : writer(std::move(writer)){
    tag_buffer    = std::make_unique<AVBuffer>(15, false);
    previous_size = 0;
}

error_t FlvAVMuxer::write_header(PSpsAVPacket& buffer) {
    static const uint8_t flv_header [] = {
            'F', 'L', 'V',
            0x01,  // version
            0x05,     // FLV_HEADER_FLAG_HASVIDEO | FLV_HEADER_FLAG_HASAUDIO
            0x00, 0x00, 0x00, 0x04     // HEADER size
    };

    error_t ret = SUCCESS;

    if (buffer) {
        if (buffer->size() != 9) {
            sp_error("flv header not 9 size");
            return ERROR_FLV_PROBE;
        }
    } else {
        if ((ret = writer->write((char *) flv_header, 9)) != SUCCESS) {
            sp_error("writer flv header fail ret: %d", ret);
            return ret;
        }
    }

    return ret;
}

error_t FlvAVMuxer::write_message(PSpsAVPacket& buffer) {
    tag_buffer->clear();
    SpsBytesWriter io(tag_buffer);

    error_t ret      = SUCCESS;
    uint8_t tag_type = 0;

    switch (buffer->stream_type) {
        case AV_STREAM_TYPE_VIDEO:
            tag_type = 9;
            break;
        case AV_STREAM_TYPE_AUDIO:
            tag_type = 8;
            break;
        case AV_STREAM_TYPE_SUBTITLE:
            tag_type = 18;
            break;
        default:
            sp_error("flv tag unknown %d", tag_type);
            return ERROR_FLV_UNKNOWN_TAG_TYPE;
    }
    io.write_int8(tag_type); // tagtype
    uint8_t* data_size_pos = tag_buffer->end();
    io.skip(3);   // skip 3-bytes data size
    io.write_int24(buffer->dts >> 8);  // low 3-bytes
    io.write_int8(buffer->dts >> 24);  // high 1-bytes
    io.write_int24(0); // stream_id

    return ret;
}

error_t FlvAVMuxer::write_tail(PSpsAVPacket& buffer) {
    return SUCCESS;
}

FlvAVOutputFormat::FlvAVOutputFormat() : IAVOutputFormat("flv", "flv") {
}

PIAVMuxer FlvAVOutputFormat::_create(PIWriter pw) const {
    return std::make_shared<FlvAVMuxer>(std::move(pw));
}

}
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

#include <memory>
#include <utility>

#include <sps_io_bytes.hpp>
#include <log/sps_log.hpp>

namespace sps {

FlvAVMuxer::FlvAVMuxer(PIWriter writer) : writer(std::move(writer)) {
    // 4 previous + 11 tag_size + 10 tmp size
    tag_buffer    = std::make_unique<AVBuffer>(15 + 4 + 10, false);
    previous_size = 0;
}

error_t FlvAVMuxer::write_header(PSpsAVPacket& buffer) {
    static const uint8_t flv_header[] = {
            'F', 'L', 'V',
            0x01,  // version
            0x05,     // FLV_HEADER_FLAG_HASVIDEO | FLV_HEADER_FLAG_HASAUDIO
            0x00, 0x00, 0x00, 0x09     // HEADER size
    };

    error_t ret = SUCCESS;

    if (buffer) {
        if (buffer->size() != 9) {
            sp_error("flv header not 9 size");
            return ERROR_FLV_PROBE;
        }
        if ((ret = writer->write(buffer->buffer(), 9)) != SUCCESS) {
            sp_error("writer flv header fail ret: %d", ret);
            return ret;
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
    SpsBytesWriter head_writer(tag_buffer);

    error_t  ret      = SUCCESS;
    uint8_t  tag_type = 0;
    uint32_t data_size    = buffer->size();
    uint32_t tmp_previous = previous_size;

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

    if (buffer->stream_type == AV_STREAM_TYPE_VIDEO && filter_video) {
        return ret;
    }

    if (buffer->stream_type == AV_STREAM_TYPE_AUDIO && filter_audio) {
        return ret;
    }

    if (buffer->stream_type == AV_STREAM_TYPE_SUBTITLE && filter_metadata) {
        return ret;
    }

    head_writer.write_int32(previous_size);        // previous size

    head_writer.write_int8(tag_type);              // tagtype
    uint8_t* data_size_pos = tag_buffer->end();
    head_writer.skip(3);                        // skip 3-bytes data size
    head_writer.write_int24(buffer->dts &  0x00FFFFFF);  // low 3-bytes
    head_writer.write_int8((buffer->dts & 0xFF000000) >> 24);  // high 1-bytes
    head_writer.write_int24(0);  // stream_id

    previous_size = 1 + 3 + 4 + 3;                 // 11 tag_size
#define WRITE_DATA_SIZE(pos, n) \
     *pos      = n >> 16;\
     *(pos+1)  = (n & 0XFFFF) >> 8;\
     *(pos+2)  = n & 0XFF;

    if (buffer->stream_type == AV_STREAM_TYPE_SUBTITLE) {
    } else if (buffer->stream_type == AV_STREAM_TYPE_AUDIO) {
        // (10. aac 14. mp3) high 4-bits
        uint8_t codecid = (buffer->flags & 0xf0) >> 4;

        head_writer.write_int8(buffer->flags);
        head_writer.write_int8(buffer->pkt_type.pkt_type);
        data_size += 2;
    } else if (buffer->stream_type == AV_STREAM_TYPE_VIDEO) {
        // 1 keyframe 2. inner 3. h263 disposable 4.
        uint8_t frame_type  = buffer->flags & 0xf0;
        uint8_t codecid     = buffer->flags & 0x0f;    // low 4-bits
        head_writer.write_int8(buffer->flags);

        data_size += 1;
        if (frame_type != 0x50) {
            // 0. avc sequence header 1. avc data 2. avc end data
            head_writer.write_int8(buffer->pkt_type.pkt_type);
            head_writer.write_int24(buffer->pts-buffer->dts);
            data_size += 4;
        }
    } else {
        ret = ERROR_FLV_UNKNOWN_TAG_TYPE;
    }


    WRITE_DATA_SIZE(data_size_pos, data_size)
    ret = writer->write(tag_buffer->buffer(), tag_buffer->size());
    if (ret != SUCCESS) {
        sp_error("fail write tag head ret:%d", ret);
        return ret;
    }


    ret = writer->write(buffer->buffer(), buffer->size());
    if (ret != SUCCESS) {
        sp_error("fail write tag data ret:%d", ret);
        return ret;
    }

    sp_debug("packet stream: %u, pkt: %d, flag: %2X, previous: %10u, "
             "data_size: %10.u, dts: %11lld, pts: %11lld, cts: %11d",
             buffer->stream_type, buffer->pkt_type.pkt_type, buffer->flags,
             tmp_previous,
             data_size,
             buffer->dts, buffer->pts, (int) (buffer->pts - buffer->dts));

    previous_size += data_size;

    return ret;
}

error_t FlvAVMuxer::write_tail(PSpsAVPacket& buffer) {
    return SUCCESS;
}

}  // namespace sps

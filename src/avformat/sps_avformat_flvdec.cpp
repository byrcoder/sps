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
#include <log/sps_log.hpp>

namespace sps {

static const int max_len = 1 * 1024 * 1024;

FlvDemuxer::FlvDemuxer(PIReader rd) {
    buf       = std::make_unique<AVBuffer>(max_len, true);
    this->io  = std::move(rd);
    this->rd  = std::make_unique<SpsBytesReader>(io, buf);
}

error_t FlvDemuxer::read_header(PSpsAVPacket &buffer) {
    auto ret = rd->acquire(9);

    if (ret != SUCCESS) {
        return ret;
    }
    uint8_t* pos = this->buf->pos();

    if (pos[0] != 'F' || pos[1] != 'L' || pos[2] != 'V') {
        sp_error("Not flv head %.*s", 9, pos);
        return ERROR_FLV_PROBE;
    }

    // version buf[3] skip

    int flags = pos[4] & (FLV_HEADER_FLAG_HASVIDEO | FLV_HEADER_FLAG_HASAUDIO); // flags
    // header_len buf[5...8]

    buffer = SpsAVPacket::create(SpsMessageType::AV_MESSAGE_HEADER,
                                 AV_STREAM_TYPE_NB,
                                 SpsAVPacketType {},
                                 pos, 9,
                                 0, 0, flags, 0);
    buffer->flags = pos[4];

    rd->skip_bytes(9);

    return ret;
}

error_t FlvDemuxer::read_packet(PSpsAVPacket& buffer) {
    const int flv_head_len = 15;

    error_t  ret    = SUCCESS;
    int      nread  = 0;
    uint32_t previous_size = 0;
    uint32_t pos           = 0;
    uint32_t tag_type      = 0;
    int      data_size     = 0;
    int64_t  dts           = 0;
    int64_t  pts           = 0;
    uint32_t stream_id     = 0;
    uint32_t flags         = 0;
    uint32_t codecid       = 0;
    int32_t  cts           = 0;
    int      pkt_type      = 0;

    if ((ret = rd->acquire(flv_head_len)) != SUCCESS) {
        sp_error("flv head not enough %d", ret);
        return ret;
    }
    previous_size = rd->read_int32(); // previous
    tag_type      = rd->read_int8();  // tag_type
    data_size     = rd->read_int24(); // data_size
    dts           = rd->read_int24(); // timestamp low 24 bit
    dts          |= ((uint32_t) rd->read_int8() << 24u); // high 8 bit
    stream_id     = rd->read_int24(); // streamid
    SpsAVStreamType stream_type = AV_STREAM_TYPE_NB;

    if ((ret = rd->acquire(data_size)) != SUCCESS) {
        sp_error("flv head not enough %d", ret);
        return ret;
    }

    sp_debug("flv tag head previous_size: %u, tag_type: %u, data_size: %u, "
            "dts:%lld, stream_id: %u",
            previous_size, tag_type, data_size,
            dts, stream_type);

    switch (tag_type) {
        case 8:
            stream_type = AV_STREAM_TYPE_AUDIO;
            break;
        case 9:
            stream_type = AV_STREAM_TYPE_VIDEO;
            break;
        case 18:
            stream_type = AV_STREAM_TYPE_SUBTITLE;
            break;
        default:
            sp_error("unknown flag: %u", tag_type);
            return ERROR_FLV_UNKNOWN_TAG_TYPE;
    }

    sp_debug("packet stream: %u, pkt: %d, flag: %2X, previous: %10u, data_size: %10.u, ",
            stream_type, pkt_type, flags, previous_size, data_size);

    if (stream_type == AV_STREAM_TYPE_AUDIO) {
        flags   = rd->read_int8();
        // channels    = flags & 0x01    // (0. SndMomo 1. SndStero)
        // sample_size = flags & 0x02    // (0. 8       1. 16)
        // sample_rate = flags & 0x0c    // (0. 5.5k    1. 11k   2. 22k   3. 44k)
        codecid = (flags & 0xf0) >> 4;   // (10. aac 14. mp3) high 4-bits

        if (codecid != SpsAudioCodec::AAC) {
            sp_error("unknown audio codecid: %d", codecid);
            return ERROR_FLV_AUDIO_CODECID;
        }
        pkt_type    =    rd->read_int8(); // 0. sequence header 1. aac data
        data_size   -=   2;
        pts         = dts;
    } else if(stream_type == AV_STREAM_TYPE_VIDEO) {
        flags       = rd->read_int8();
        uint8_t frame_type = flags & 0xf0 ;   // 1 keyframe 2. inner 3. h263 disposable 4.
        codecid     = flags & 0x0f;           // low 4-bits

        if (codecid != SpsVideoCodec::H264 && codecid != SpsVideoCodec::H265) {
            sp_error("unknown video codecid: %d", codecid);
            return ERROR_FLV_VIDEO_CODECID;
        }
        --data_size;

        if (frame_type != 0x50) {
            pkt_type = rd->read_int8(); // 0. avc sequence header 1. avc data 2. avc end data
            cts = rd->read_int24();
            cts = (cts + 0xff800000) ^ 0xff800000; // if cts < 0 convert to int32_t
            pts = dts + cts;
            data_size = data_size - 4;
        }
        sp_debug("frametype: %2X, codecid: %2X, dts: %12lld, pts: %12lld, cts: %11d, "
                "data_size: %11u",
                frame_type, codecid, dts, pts, cts, data_size);
    }

    if (data_size <= 0) {
        sp_error("data size is empty");
        return ERROR_FLV_TAG;
    }

    buffer = SpsAVPacket::create(SpsMessageType::AV_MESSAGE_DATA,
                                 stream_type,
                                 SpsAVPacketType{pkt_type},
                                 buf->pos(), data_size,
                                 dts, pts, flags, codecid);

    rd->skip_bytes(data_size);
    sp_info("packet stream: %u, pkt: %d, flag: %2X, previous: %10u, data_size: %10.u, "
            "dts: %11lld, pts: %11lld, cts: %11d",
            stream_type, pkt_type, flags, previous_size, data_size,
            dts, pts, cts);

    return ret;
}

error_t FlvDemuxer::read_tail(PSpsAVPacket& buffer) {
    return SUCCESS;
}

error_t FlvDemuxer::probe(PSpsAVPacket& buffer) {
    auto pos = buffer->buffer();
    int  len = buffer->size();

    if (len < 3 || pos[0] != 'F' || pos[1] != 'L' || pos[2] != 'V') {
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
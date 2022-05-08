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
// Created by byrcoder on 2021/7/4.
//

#include <sps_avformat_rtmpenc.hpp>

#include <utility>

#include <sps_log.hpp>
#include <librtmp/sps_librtmp.hpp>

namespace sps  {

RtmpAVMuxer::RtmpAVMuxer(PIWriter writer) {
    this->writer = std::dynamic_pointer_cast<RtmpUrlProtocol>(
            std::move(writer));
}

error_t RtmpAVMuxer::write_header(PAVPacket& buffer) {
    return SUCCESS;
}

// TODO(byrcoder): FIXME
error_t RtmpAVMuxer::write_packet(PAVPacket& buffer) {
    WrapRtmpPacket packet(false);
    auto& pkt = packet.packet;

#if 0
    // TODO: FIXME (rtmp buffer copy)
    // RTMP_SendPacket will change the buffer when send with chunked size
    auto copy_buffer = SpsAVPacket::create(
            buffer->msg_type, buffer->stream_type, buffer->pkt_type,
            buffer->buffer(), buffer->size(), buffer->dts, buffer->pts,
            buffer->flags, buffer->codecid, buffer->duration
            );
    copy_buffer->number = buffer->number;
    buffer = std::move(copy_buffer);
#endif

    error_t  ret          = SUCCESS;
    int      head_len     = buffer->head_size();
    uint8_t* head_pos     = buffer->head_pos();
    pkt.m_body            = (char*) buffer->buffer();
    pkt.m_nBodySize       = buffer->size();
    pkt.m_hasAbsTimestamp = 1;
    pkt.m_nTimeStamp      = buffer->dts;
    pkt.m_nInfoField2     = 1;

    if (buffer->is_script()) {
        pkt.m_headerType = RTMP_PACKET_SIZE_LARGE;
        pkt.m_packetType = RTMP_PACKET_TYPE_INFO;

        pkt.m_nChannel   = 4;

        buffer->debug();
        sp_info("%d. rtmp encode script stream pkt: 0x%2X, flag: %2X, "
                "data_size: %10.d, dts: %11d, pts: %1d, cts: %1d",
                buffer->number, pkt.m_packetType, 0, pkt.m_nBodySize,
                pkt.m_nTimeStamp, 0, 0);
    } else if (buffer->is_audio()) {
        pkt.m_packetType = RTMP_PACKET_TYPE_AUDIO;
        pkt.m_headerType = RTMP_PACKET_SIZE_LARGE;

        pkt.m_nChannel = 4;
        if (head_len < 2) {
            sp_error("rtmp head len %d < 2", head_len);
            return ERROR_RTMP_HEAD_TOO_SHORT;
        }

        // 0. sequence header 1. aac data
        *--pkt.m_body  = buffer->is_audio_sequence_header() ? 0 : 1;
        *--pkt.m_body = buffer->flags;
        pkt.m_nBodySize += 2;

        sp_debug("rtmp encode audio stream pkt: %d, flag: %2X, "
                 "data_size: %10.d(%d), dts: %11d, pts: %lld, cts: %lld",
                pkt.m_packetType, 0, pkt.m_nBodySize, buffer->size(),
                pkt.m_nTimeStamp, buffer->dts, buffer->dts);

    } else if (buffer->is_video()) {
        pkt.m_packetType = RTMP_PACKET_TYPE_VIDEO;
        pkt.m_headerType = RTMP_PACKET_SIZE_LARGE;
        pkt.m_nChannel = 6;

        // 0x10. keyframe
        // 0x20. inner
        // 0x30. h263 disposable 0x40. 0x50. frame info or order frame
        uint8_t frame_type  = buffer->flags & 0xf0;

        if (head_len < 5) {
            sp_error("rtmp head len %d < 5", head_len);
            return ERROR_RTMP_HEAD_TOO_SHORT;
        }

        if (frame_type != 0x50) {
            int32_t cts = buffer->pts - buffer->dts;
            // if cts < 0 convert to int32_t
            cts         = (cts + 0xff800000) ^ 0xff800000;

            *(--pkt.m_body) = cts & 0x0f;
            *(--pkt.m_body) = cts & 0xf0;
            *(--pkt.m_body) = cts & 0x0f00;

            *(--pkt.m_body) = buffer->is_video_sequence_header() ? 0 : 1;
            pkt.m_nBodySize += 4;
        }

        *(--pkt.m_body) =  buffer->flags;
        pkt.m_nBodySize += 1;

        if (buffer->is_keyframe()) {
            sp_info("%d. rtmp encode video stream pkt: %d, flag: %2X, "
                    "data_size: %10.d (%d), dts: %11d, pts: %lld, cts: %lld",
                     buffer->number, pkt.m_packetType, buffer->flags, pkt.m_nBodySize,
                     buffer->size(), pkt.m_nTimeStamp, buffer->dts,
                     buffer->pts - buffer->dts);
        }
    } else {
        sp_warn("unknown message type %d", buffer->stream_type);
        return SUCCESS;
    }

    if ((ret = writer->write(packet)) != SUCCESS) {
        sp_error("fail write rtmp packet ret %d", ret);
        return ret;
    }

    return SUCCESS;
}

error_t RtmpAVMuxer::write_tail(PAVPacket& buffer) {
    return SUCCESS;
}

}  // namespace sps

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
// Created by byrcoder on 2021/7/3.
//

#include <sps_avformat_rtmpdec.hpp>

#include <memory>

#include <sps_io_memory.hpp>
#include <sps_log.hpp>

namespace sps {

RtmpDemuxer::RtmpDemuxer(PIReader rd) {
    this->io  = std::dynamic_pointer_cast<RtmpUrlProtocol>(rd);
}

error_t RtmpDemuxer::read_header(PAVPacket& buffer) {
    return SUCCESS;
}

error_t RtmpDemuxer::read_packet(PAVPacket& buffer) {
    if (!io) {
        sp_error("fatal rtmp io is nullptr");
        return ERROR_AVFORMAT_RTMP_IO_NULL;
    }

    error_t ret = SUCCESS;

    uint8_t flag       = 0;
    // 1 keyframe 2. inner 3. h263 disposable 4. 5. frame info or order frame
    uint8_t frame_type = 0;
    uint8_t codecid    = 0;           // low 4-bits
    size_t  data_size  = 0;
    uint8_t pkt_type   = 0;
    int64_t  dts       = 0;
    int64_t  pts       = 0;
    int32_t  cts       = 0;
    AVStreamType stream_type = AV_STREAM_TYPE_NB;

    do {
        WrapRtmpPacket packet;
        ret = io->read(packet);

        if (ret != SUCCESS) {
            sp_error("fail read rtmp ret %d", ret);
            return ret;
        }

        auto av_buffer = std::make_shared<AVBuffer>(
                packet.data(), packet.size(), packet.size(), true);
        PIReader null_reader;
        BytesReader stream(null_reader, av_buffer);

        data_size  = packet.size();
        pkt_type   = 0;
        dts        = packet.packet.m_nTimeStamp;
        pts        = dts;
        cts        = 0;

        if (packet.is_video()) {
            if ((ret = stream.acquire(1)) != SUCCESS) {
                sp_error("fail too short rtmp video %lu, ret %d",
                          packet.size(), ret);
                return ret;
            }

            flag        = stream.read_int8();
            // 0x10. keyframe
            // 0x20. inner
            // 0x30. h263 disposable
            // 0x40. 0x50. frame info or order frame
            frame_type  = *packet.data() & 0xf0;
            codecid     = *packet.data() & 0x0f;    // low 4-bits
            stream_type = AV_STREAM_TYPE_VIDEO;

            if (codecid != VideoCodec::H264
                    && codecid != VideoCodec::H265) {
                sp_error("unknown video codecid: %d", codecid);
                return ERROR_FLV_VIDEO_CODECID;
            }
            --data_size;

            if (frame_type != 0x50) {
                if ((ret = stream.acquire(4)) != SUCCESS) {
                    sp_error("fail too short rtmp video %lu, ret %d",
                              packet.size(), ret);
                    return ret;
                }

                // 0. avc sequence header 1. avc data 2. avc end data
                pkt_type = stream.read_int8();
                cts      = stream.read_int24();

                // if cts < 0 convert to int32_t
                cts      = (cts + 0xff800000) ^ 0xff800000;
                pts      = dts + cts;
                data_size = data_size - 4;
            }

#if 0
            if (frame_type == 0x10) {
                sp_debug("got keyframe dts: %lld", dts);
            }
#endif

            assert(data_size == av_buffer->size());
            buffer = AVPacket::create(AVMessageType::AV_MESSAGE_DATA,
                                      AV_STREAM_TYPE_VIDEO,
                                      AVPacketType{pkt_type},
                                      av_buffer->pos(), av_buffer->size(),
                                      dts, pts, flag, codecid);
            break;
        } else if (packet.is_audio()) {
            if ((ret = stream.acquire(1)) != SUCCESS) {
                sp_error("fail too short rtmp audio %lu, ret %d",
                          packet.size(), ret);
                return ret;
            }

            flag   = stream.read_int8();
            // channels    = flags & 0x01    // (0. SndMomo 1. SndStero)
            // sample_size = flags & 0x02    // (0. 8       1. 16)
            // sample_rate = flags & 0x0c    // (0. 5.5k 1. 11k  2. 22k 3. 44k)
            codecid = (flag & 0xf0) >> 4;   // (10. aac 14. mp3) high 4-bits
            stream_type = AV_STREAM_TYPE_AUDIO;

            if (codecid != AudioCodec::AAC) {
                sp_error("unknown audio codecid: %d", codecid);
                return ERROR_FLV_AUDIO_CODECID;
            }

            // 0. sequence header 1. aac data
            pkt_type    =    stream.read_int8();
            data_size   -=   2;
            pts         =    dts;

            assert(data_size == av_buffer->size());
            buffer = AVPacket::create(AVMessageType::AV_MESSAGE_DATA,
                                      AV_STREAM_TYPE_AUDIO,
                                      AVPacketType{pkt_type},
                                      av_buffer->pos(), av_buffer->size(),
                                      dts, pts, flag, codecid);
            break;
        } else if (packet.is_script()) {
            stream_type = AV_STREAM_TYPE_SUBTITLE;
            assert(data_size == av_buffer->size());
            buffer = AVPacket::create(AVMessageType::AV_MESSAGE_DATA,
                                      AV_STREAM_TYPE_SUBTITLE,
                                      AVPacketType{0},
                                      av_buffer->pos(), av_buffer->size(),
                                      dts, pts, flag, codecid);
            break;
        } else {
            sp_info("skip pkt type %d", packet.packet.m_packetType);
        }
    } while (true);

    sp_debug("packet stream: %u, pkt: %d, flag: %2X, data_size: %10.lu, "
            "dts: %11lld, pts: %11lld, cts: %11d",
            stream_type, pkt_type, flag, data_size,
            dts, pts, cts);

    return ret;
}

error_t RtmpDemuxer::read_tail(PAVPacket& buffer) {
    return SUCCESS;
}

error_t RtmpDemuxer::probe(PAVPacket &buffer) {
    return ERROR_RTMP_NOT_IMPL;
}

}  // namespace sps

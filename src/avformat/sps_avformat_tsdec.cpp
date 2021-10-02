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
// Created by byrcoder on 2021/7/23.
//

#include <sps_avformat_tsdec.hpp>

#include <utility>

#include <sps_io_bytes.hpp>

#include <sps_log.hpp>

namespace sps {

TsCache::TsCache(StreamCache::PICacheStream cache) {
    this->cache = std::move(cache);
}

error_t TsCache::on_packet(uint8_t *buf, size_t len,
        TsProgramType ts_type, int payload_unit_start_indicator) {
    auto pkt = AVPacket::create(AV_MESSAGE_DATA,
            ts_type == TS_PROGRAM_PES ? AV_STREAM_PES : AV_STREAM_PSI,
            AVPacketType{ts_type | (payload_unit_start_indicator >> 31)},
            buf,
            len, 0, 0);
    return cache->put(pkt);
}

TsDemuxer::TsDemuxer(PIReader rd, PITsPacketHandler ph) : ts_ctx(this, ph) {
    buf       = std::make_shared<AVBuffer>(SPS_TS_PACKET_SIZE, false);
    this->rd  = std::make_unique<BytesReader>(rd, buf);

    // TODO: FIXME more than h264
    video_codec_parser = std::make_shared<H264AVCodecParser>();
    audio_codec_parser = std::make_shared<AacAVCodecParser>();
}

error_t TsDemuxer::read_header(PAVPacket & buffer) {
    return SUCCESS;
}

error_t TsDemuxer::read_packet(PAVPacket& buffer) {
    error_t ret = SUCCESS;

    do {
        if (pop(buffer)) {
            return SUCCESS;
        }

        if ((ret = rd->acquire(SPS_TS_PACKET_SIZE)) != SUCCESS) {
            sp_error("fail read ts packet ret %d", ret);
            return ret;
        }

        sp_debug("get packet size %lu(%c), (%lu~%lu)",
                buf->size(), *buf->buffer(), buf->buf_ptr, buf->buf_end);

        if ((ret = decode_packet(buf->buffer(), SPS_TS_PACKET_SIZE)) != SUCCESS) {
            sp_error("fail decode ts packet ret %d", ret);
            return ret;
        }

        rd->buf->clear();

    } while (true);

    return ret;
}

error_t TsDemuxer::read_tail(PAVPacket& buffer) {
    return SUCCESS;
}

error_t TsDemuxer::probe(PAVPacket& buffer) {
    return ERROR_STREAM_NOT_IMPL;
}

error_t TsDemuxer::decode_packet(uint8_t* pkt, size_t len) {
    int pos = 0;
    error_t ret = SUCCESS;

    while (pos < len) {
        if (len - pos < SPS_TS_PACKET_SIZE) {
            ret = ERROR_TS_PACKET_SIZE;
            sp_error("fail read ts packet ret %d", ret);
            return ret;
        }

        if ((ret = ts_ctx.decode(pkt + pos, SPS_TS_PACKET_SIZE)) != SUCCESS) {
            sp_error("fail decode ts packet ret %d", ret);
            return ret;
        }

        pos += SPS_TS_PACKET_SIZE;
    }

    return ret;
}

bool TsDemuxer::pop(PAVPacket &pkt) {
    if (!encoder_pkts.empty()) {
        pkt = encoder_pkts.front();
        encoder_pkts.erase(encoder_pkts.begin());
        return true;
    }

    return false;
}

error_t TsDemuxer::on_pes_complete(TsPesContext *pes) {
    error_t ret = SUCCESS;

    switch (pes->stream_type) {
        case TS_STREAM_TYPE_VIDEO_H264:
            ret = on_h264(pes);
            break;

        case TS_STREAM_TYPE_AUDIO_AAC:
            ret = on_aac(pes);
            break;
        case TS_STREAM_TYPE_VIDEO_H265:

            break;
        default:
            sp_warn("ignore stream_type %x", pes->stream_type);
    }
    return ret;
}

error_t TsDemuxer::on_h264(TsPesContext* pes) {
    error_t ret = SUCCESS;
    std::list<PAVPacket> pkts;

    sp_debug("h264 pes stream type %x, size %u", pes->stream_type,
             pes->pes_packet_length);

    AVCodecContext ctx(pes->dts, pes->pts, 90);
    ret = video_codec_parser->encode_avc(&ctx, pes->pes_packets->buffer(),
                                         pes->pes_packet_length, pkts);

    if (ret != SUCCESS) {
        sp_error("fail encode video ret %d", ret);
        return ret;
    }

    this->encoder_pkts.insert(encoder_pkts.end(), pkts.begin(), pkts.end());
    sp_debug("send %d video pkt", (int) pkts.size());

    return ret;
}

error_t TsDemuxer::on_aac(TsPesContext* pes) {
    error_t ret = SUCCESS;
    std::list<PAVPacket> pkts;

    sp_debug("aac pes stream type %x, size %u", pes->stream_type,
            pes->pes_packet_length);

    // return ret;

    AVCodecContext ctx(pes->dts, pes->pts, 90);
    ret = audio_codec_parser->encode_avc(&ctx, pes->pes_packets->buffer(),
                                         pes->pes_packets->size(), pkts);

    if (ret != SUCCESS) {
        sp_error("fail encode video ret %d", ret);
        return ret;
    }

    this->encoder_pkts.insert(encoder_pkts.end(), pkts.begin(), pkts.end());
    sp_debug("send %d audio pkt", (int) pkts.size());

    return ret;
}

}  // namespace sps

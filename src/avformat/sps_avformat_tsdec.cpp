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

TsDemuxer::TsDemuxer(PIReader rd) : ts_ctx(this) {
    buf       = std::make_shared<AVBuffer>(SPS_TS_PACKET_SIZE, false);
    this->rd  = std::make_unique<BytesReader>(rd, buf);
}

error_t TsDemuxer::read_header(PAVPacket & buffer) {
    return SUCCESS;
}

error_t TsDemuxer::read_packet(PAVPacket& buffer) {
    error_t ret = SUCCESS;

    if (!encoder_pkts.empty()) {
        buffer = encoder_pkts.front();
        encoder_pkts.erase(encoder_pkts.begin());
        return SUCCESS;
    }

    do {
        if ((ret = rd->acquire(SPS_TS_PACKET_SIZE)) != SUCCESS) {
            sp_error("fail read ts packet ret %d", ret);
            return ret;
        }

        sp_debug("get packet size %lu(%c), (%lu~%lu)",
                buf->size(), *buf->buffer(), buf->buf_ptr, buf->buf_end);

        if ((ret = ts_ctx.decode(buf->buffer(), SPS_TS_PACKET_SIZE)) != SUCCESS) {
            sp_error("fail decode ts packet ret %d", ret);
            return ret;
        }

        rd->buf->clear();

        if (!encoder_pkts.empty()) {
            buffer = encoder_pkts.front();
            encoder_pkts.erase(encoder_pkts.begin());
            return SUCCESS;
        }

    } while (true);

    return ret;
}

error_t TsDemuxer::read_tail(PAVPacket& buffer) {
    return SUCCESS;
}

error_t TsDemuxer::probe(PAVPacket& buffer) {
    return ERROR_STREAM_NOT_IMPL;
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

    sp_info("pes stream type %x, size %u",
             pes->stream_type, pes->pes_packet_length);

    NALUContext nalus(pes->dts, pes->pts);
    ret = nalu_decoder.decode(pes->pes_packets->buffer(),
                        pes->pes_packet_length,
                        &nalus,
                        false);

    if (ret != SUCCESS) {
        sp_error("fail decode video ret %d", ret);
        return ret;
    }

    std::list<PAVPacket> pkts;
    ret = nalu_encoder.encode_avc(&nalus, pkts);

    if (ret != SUCCESS) {
        sp_error("fail encoder video ret %d", ret);
        return ret;
    }

    this->encoder_pkts.insert(encoder_pkts.end(), pkts.begin(), pkts.end());
    sp_info("send %d video pkt", (int) pkts.size());

    return ret;
}

error_t TsDemuxer::on_aac(TsPesContext* pes) {
    error_t ret = SUCCESS;

    return ret;
}

}  // namespace sps

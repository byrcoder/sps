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
// Created by byrcoder on 2022/5/12.
//

#include <sps_url_rtmp_ffmpeg.hpp>
#include <sps_io_bytes.hpp>

#include <sps_log.hpp>
#include <sps_util_time.hpp>

namespace sps {

error_t parser_flv_head(uint8_t* buf, int size, uint8_t* tag_type, uint32_t* data_size,
                       uint32_t* dts, uint32_t* stream_id) {
    if (size < FLV_TAG_SIZE) {  // 11
        return ERROR_IO_NOT_ENOUGH;
    }

    auto rd = BytesReader::create_reader(buf, size);

    *tag_type      = rd->read_int8();   // tag_type
    *data_size     = rd->read_int24();  // data_size
    *dts           = rd->read_int24();  // timestamp low 24 bit
    *dts          |= ((uint32_t) rd->read_int8() << 24u);  // high 8 bit
    *stream_id     = rd->read_int24();  // streamid

    return SUCCESS;
}

// append_flv_data
error_t write_flv_tag(uint8_t* buf, int size, uint32_t previous_size,
                      uint8_t tag_type, uint32_t data_size,
                      uint32_t dts, uint32_t stream_id) {
    if (size < FLV_TAG_SIZE) {  // 11
        return ERROR_IO_NOT_ENOUGH;
    }

    auto rd = BytesWriter::create_writer(buf, size);

    // rd->write_int32(previous_size);
    rd->write_int8(tag_type);
    rd->write_int24(data_size);
    rd->write_int24(dts & 0x00FFFFFF);
    rd->write_int8(dts  >> 24);
    rd->write_int24(stream_id);

    return SUCCESS;
}


FFmpegRtmpUrlProtocol::FFmpegRtmpUrlProtocol(PRtmpHook hk) {
    this->hk = std::move(hk);
}

error_t FFmpegRtmpUrlProtocol::open(PRequestUrl &url, Transport p) {
    error_t        ret   = SUCCESS;
    auto           ip    = (url->get_ip().empty()) ? (url->get_host()) :
                                                     (url->get_ip());
    p                    = (p == Transport::DEFAULT ? Transport::TCP : p);

    auto socket = SingleInstance<ClientSocketFactory>::get_instance().
                      create_ss(p, ip, url->get_port(), url->get_timeout());

    if (!socket) {
        sp_error("failed connect %s:%d, type: %d", ip.c_str(), url->get_port(), p);
        return ERROR_HTTP_SOCKET_CONNECT;
    }

    auto full_url = url->schema + "://" + url->host + url->url;
    hk            = std::make_shared<RtmpHook>(socket);
    ret           = hk->client_connect(full_url, url->params, false);

    sp_info("rtmp open %s?%s ret %d", full_url.c_str(), url->params.c_str(), ret);
    return ret;
}

error_t FFmpegRtmpUrlProtocol::read(void *buf, size_t size, size_t &nread) {
    static const char flv_header[] = {'F', 'L', 'V', 0x01,
                                      0x05,				/* 0x04 == audio, 0x01 == video */
                                      0x00, 0x00, 0x00, 0x09,
                                      0x00, 0x00, 0x00, 0x00   /* previous_size 0 */
    };

    error_t ret = SUCCESS;
    uint8_t* p  = (uint8_t*) buf;
    size_t   r  = size;

    if (!flv_head_read) {
        if (size < FLV_HEAD_SIZE) {
            return ERROR_IO_NOT_ENOUGH;
        }

        flv_head_read = true;
        memcpy(buf, flv_header, FLV_HEAD_SIZE);
        nread = FLV_HEAD_SIZE;

        sp_info("FLV HEAD %s", to_hex(flv_header, sizeof(flv_header)).c_str());

        return ret;
    }

    while (!pkt.data() ||
           (pkt.size() + FLV_TAG_SIZE + 4 == pkt_offset)) {  // tag 11 + size + previous size 4
        pkt_offset = 0;
        pkt.reset();
        if ((ret = hk->recv_packet(pkt)) != SUCCESS) {
            sp_error("failed RTMP RCV packet ret %d", ret);
            return ret;
        }

        // only read video/audio/script
        // append flv
        if (pkt.is_video() || pkt.is_audio() || pkt.is_script()) {
            write_flv_tag(pkt_tag_head, sizeof(pkt_tag_head), previous_size,
                          pkt.packet.m_packetType, pkt.packet.m_nBodySize,
                          pkt.packet.m_nTimeStamp, pkt.packet.m_nInfoField2);
            sp_debug("rtmp->flv head type %d, size %u, timestamp %d, "
                    "stream_id %d, %s", pkt.packet.m_packetType, pkt.packet.m_nBodySize,
                    pkt.packet.m_nTimeStamp, pkt.packet.m_nInfoField2,
                    to_hex((char*) pkt_tag_head, 11).c_str());
            break;
        }

        pkt.reset();
    }

    // flv tag header 11
    if (size && pkt_offset < FLV_TAG_SIZE) {
        size_t n   = std::min((int32_t) size, (int32_t) (FLV_TAG_SIZE - pkt_offset));
        memcpy(p, pkt_tag_head + pkt_offset, n);
        size       -= n;
        pkt_offset += n;
        p          += n;
    }

    // flv tag data
    if (size && FLV_TAG_SIZE + pkt.size() > pkt_offset) {
        size_t n = std::min((int32_t) size, (int32_t) (pkt.size() + FLV_TAG_SIZE - pkt_offset));
        memcpy(p, pkt.data() + pkt_offset - FLV_TAG_SIZE, n);
        size       -=n;
        pkt_offset += n;
        p          += n;
    }

    // flv previous size
    if (size >= 4) {
        auto rd = BytesWriter::create_writer((uint8_t*) p, 4);
        rd->write_int32(pkt.size() + 11);
        size       -= 4;
        pkt_offset += 4;
        p          += 4;

        sp_debug("FLV tag %s, previous %s, total_size %lu", to_hex((char*) buf, 11).c_str(),
                 to_hex((char*) buf + pkt_offset - 4, 4).c_str(), p - (uint8_t*) buf);

    }

    nread = p - (uint8_t*) buf;

    return ret;
}

error_t FFmpegRtmpUrlProtocol::write(void *buf, size_t size) {
    error_t  ret          = SUCCESS;
    uint8_t  tag_type;
    uint32_t timestamp;  // 4bytes
    uint32_t data_size;  // 3bytes
    uint32_t stream_id;  // 3bytes

    if ((ret = parser_flv_head((uint8_t*)buf, size, &tag_type, &timestamp,
                               &data_size, &stream_id)) != SUCCESS) {
        return ret;
    }


    WrapRtmpPacket packet(false);
    auto& pkt             = packet.packet;

    pkt.m_body            = (char*) buf + FLV_TAG_SIZE;
    pkt.m_nBodySize       = size - FLV_TAG_SIZE;
    pkt.m_hasAbsTimestamp = 1;
    pkt.m_nTimeStamp      = timestamp;
    pkt.m_nInfoField2     = 1;
    pkt.m_headerType      = RTMP_PACKET_SIZE_LARGE;
    pkt.m_packetType      = tag_type;

    if ((ret = hk->send_packet(pkt, false)) != SUCCESS) {
        sp_error("failed RTMP RCV packet ret %d", ret);
        return ret;
    }

    return ret;
}

PResponse FFmpegRtmpUrlProtocol::response() {
    return nullptr;
}

}

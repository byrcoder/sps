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

namespace sps {

error_t parser_flv_head(uint8_t* buf, int size, uint8_t* tag_type, uint32_t* data_size,
                       uint32_t* dts, uint32_t* stream_id) {
    if (size < 1 + 3 + 3 + 1 + 3) {  // 11
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
    error_t ret = SUCCESS;

    while (pkt.size() <= pkt_offset) {
        pkt_offset = 0;
        pkt.reset();
        if ((ret = hk->recv_packet(pkt)) != SUCCESS) {
            sp_error("failed RTMP RCV packet ret %d", ret);
            return ret;
        }

        // only read video/audio/script
        if (pkt.is_video() || pkt.is_audio() || pkt.is_script()) {
            break;
        }

        pkt.reset();
    }

    nread = std::min(pkt.size() - pkt_offset, size);
    memcpy(buf, pkt.data() + pkt_offset, nread);
    pkt_offset += nread;

    return ret;
}

error_t FFmpegRtmpUrlProtocol::write(void *buf, size_t size) {
    error_t  ret          = SUCCESS;
    uint8_t  tag_type;
    uint32_t timestamp;  // 4bytes
    uint32_t data_size;  // 3bytes
    uint32_t stream_id;  // 3bytes

    if ((ret = parser_flv_head((uint8_t* )buf, size, &tag_type, &timestamp,
                               &data_size, &stream_id)) != SUCCESS) {
        return ret;
    }


    WrapRtmpPacket packet(false);
    auto& pkt             = packet.packet;

    pkt.m_body            = (char*) buf + 11;
    pkt.m_nBodySize       = size - 11;
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
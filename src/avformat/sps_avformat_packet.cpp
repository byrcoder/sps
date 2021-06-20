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
// Created by byrcoder on 2021/6/17.
//

#include <sps_avformat_packet.hpp>
#include <log/sps_log.hpp>

namespace sps {

PSpsAVPacket SpsAVPacket::create(SpsMessageType msg_type, SpsAVStreamType stream_type,
                                 SpsAVPacketType pkt_type, uint8_t* buf, int len,
                                 int64_t dts, int64_t pts, int flags, int codecid,
                                 int64_t duration) {
    auto pkt = std::make_shared<SpsAVPacket> (buf, len);
    pkt->msg_type    = msg_type;
    pkt->stream_type = stream_type;
    pkt->pkt_type    = pkt_type;

    pkt->dts         = dts;
    pkt->pts         = pts;
    pkt->flags       = flags;
    pkt->codecid     = codecid;
    pkt->duration    = duration;
    return pkt;
}

SpsAVPacket::SpsAVPacket(uint8_t *buf, int len) :  CharBuffer(buf, len) {
}

void SpsAVPacket::debug() {
    sp_append_start("msg_type: %d, stream_type: %d, pkt_type: %11d, dts: %11lld, "
            "pts: %11lld, flags: %2X, codecid: %2d, length: %10d, ",
            msg_type, stream_type, pkt_type.pkt_type, dts,
            pts, flags, codecid, size());

    for (int i = 0; i < size(); ++i) {
        sp_append("%2X", *(buffer()+i));
    }
    sp_append_end();
}

}
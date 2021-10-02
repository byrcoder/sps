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
// Created by byrcoder on 2021/10/1.
//

#include <sps_avformat_rtpdec_ts.hpp>

namespace sps {

MpegtsRtpDecodder::MpegtsRtpDecodder() {
    ts_demuxer = std::make_shared<TsDemuxer>(nullptr, nullptr);
}

bool MpegtsRtpDecodder::match(int pt) {
    return pt == RTP_TS;
}

error_t MpegtsRtpDecodder::decode(RtpPayloadHeader &header, BitContext &bc, std::list<PAVPacket> &pkts) {
    auto ret = ts_demuxer->decode_packet(bc.pos(), bc.size());

    if (ret != SUCCESS) {
        sp_error("[rtp@ts decode ret %d", ret);
        return ret;
    }

    bc.skip_read_bytes(bc.size());

    PAVPacket pkt;
    while (ts_demuxer->pop(pkt))  {
        pkts.push_back(std::move(pkt));
    }

    return ret;
}

}
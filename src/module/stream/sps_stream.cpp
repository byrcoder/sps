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
// Created by byrcoder on 2022/7/31.
//

#include <sps_stream.hpp>

namespace sps {

StreamDecoder::StreamDecoder(PIAVDemuxer demuxer, StreamCache::PICacheStream cache) :
    dec(std::move(demuxer)), cache(std::move(cache)) {
}

error_t StreamDecoder::decode() {
    error_t ret = SUCCESS;
    PAVPacket packet;

    // support ffmpeg ctx
    cache->set_ctx(dec->get_av_ctx());

    if ((ret = dec->read_header(packet)) != SUCCESS) {
        sp_error("fail read header recv ret %d", ret);
        return ret;
    }

    do {
        if ((ret = dec->read_packet(packet)) != SUCCESS) {
            sp_error("fail publishing recv ret %d", ret);
            break;
        }

        // packet->debug();
        cache->put(packet);
    } while (true);

    return ret;
}

StreamEncoder::StreamEncoder(PIAVMuxer muxer, PAVDumpCacheStream cache, bool wh) : enc(std::move(muxer)),
    cache (std::move(cache)) {
    wrote_header = wh;
}

error_t StreamEncoder::encode() {
    PAVPacket buffer;
    error_t ret = wrote_header ? enc->write_header(buffer) : SUCCESS;

    if (ret != SUCCESS) {
        sp_error("Failed encoder write header url protocol for ret:%d", ret);
        return ret;
    }

    do {
        std::list<PAVPacket> vpb;
        int n = cache->dump(vpb, false);

        if (n == 0) {
            ret = ERROR_SOCKET_TIMEOUT;
            sp_error("Fail timeout playing rcv ret %d", ret);
            break;
        }

        for (auto& p : vpb) {
            if ((ret = enc->write_packet(p)) != SUCCESS) {
                sp_error("fail encoder write message url protocol for ret:%d",
                         ret);
                break;
            }

            sp_debug("write packet!");
        }
    } while (ret == SUCCESS);

    return ret;
}

}
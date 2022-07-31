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

#include <sps_stream_upstream.hpp>

#include <sps_avformat_dec.hpp>
#include <sps_url.hpp>

namespace sps {

StreamUpStream::StreamUpStream(PRequestUrl url, std::string &server, int port,
                               StreamCache::PICacheStream cache) :
    url (std::move(url)), server (server), port(port), cache(std::move(cache)) {
    running = true;
}

error_t StreamUpStream::streaming() {
    error_t ret = SUCCESS;
    auto& avf   = SingleInstance<AVDemuxerFactory>::get_instance();
    auto  io    = SingleInstance<sps::UrlProtocol>::get_instance().create(url);

    if (!io) {
        sp_error("Fail create url %s", url->get_url());
        return ERROR_STREAM_IO_FAIL;
    }
    auto enc = avf.create(io, url);

    PAVPacket packet;
    ret = enc->read_header(packet);

    if (ret != SUCCESS) {
        sp_error("Fail read header %d", ret);
        return ret;
    }

    while (running) {
        if ((ret = enc->read_packet(packet)) != SUCCESS) {
            sp_error("Fail read packet %d", ret);
            break;
        }
        cache->put(packet);
    }

    running = false;
    return ret;
}

void StreamUpStream::stop() {
    running = false;
}

}
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

#include <app/upstream/sps_upstream.hpp>

#include <app/url/sps_url_protocol.hpp>

namespace sps {

Upstream::Upstream(PICacheStream cs, PIAvDemuxer dec, PRequestUrl url) {
    this->cs        = std::move(cs);
    this->decoder   = std::move(dec);
    this->url       = std::move(url);
    SingleInstance<UpstreamContainer>::get_instance().reg(shared_from_this());
}

Upstream::~Upstream() {
    on_stop();
    abort_request();
}

void Upstream::abort_request() {
    cs->close();
}

error_t Upstream::handler() {
    PIBuffer header;
    error_t ret = decoder->read_header(header);
    if (ret != SUCCESS) {
        sp_error("Failed read head:%d", ret);
        return ret;
    }

    cs->put(header);
    do {
        PIBuffer body;
        if ((ret = decoder->read_message(body)) != SUCCESS) {
            break;
        }
        cs->put(body);
    } while(true);

    if (ret != ERROR_STREAM_EOF) {
        sp_error("Failed read body:%d", ret);
        return ret;
    }

    PIBuffer tail;
    if ((ret = decoder->read_tail(tail)) != SUCCESS) {
        sp_error("Failed read tail:%d", ret);
        return ret;
    }
    cs->put(tail);

    return ret;
}

void Upstream::on_stop() {
    SingleInstance<UpstreamContainer>::get_instance().cancel(shared_from_this());
}

PICacheStream Upstream::get_cs() {
    return cs;
}

PIUpstream UpstreamFactory::create_upstream(PICacheStream cs, PRequestUrl url) {
    auto p = SingleInstance<UrlProtocol>::get_instance().create(url);
    if (!p) {
        return nullptr;
    }

    if (p->open(url, DEFAULT) != SUCCESS) {
        return nullptr;
    }

    auto decoder = SingleInstance<AvDemuxerFactory>::get_instance().create(p, url);
    if (!decoder) {
        return nullptr;
    }
    return std::make_shared<Upstream>(cs, decoder, url);
}

}

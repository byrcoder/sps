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

#include <sps_stream_edge.hpp>

namespace sps {

StreamEdge::StreamEdge(const std::string& key, PRequestUrl url, const std::string& server, int port,
                       StreamCache::PICacheStream cache) : cache(std::move(cache)), server(server) {
    this->key  = key;
    this->port = port;
    this->url  = std::move(url);
    started    = false;
}

error_t StreamEdge::handler() {
    if (!started) {
        sp_error("stopped");
        return SUCCESS;
    }

    error_t ret = SUCCESS;
    auto& avf   = SingleInstance<AVDemuxerFactory>::get_instance();
    auto  io    = SingleInstance<sps::UrlProtocol>::get_instance().create(url);

    if (!io) {
        sp_error("Fail create url %s", url->get_url());
        return ERROR_STREAM_IO_FAIL;
    }
    auto dec = avf.create(io, url);
    decoder  = std::make_shared<StreamDecoder>(dec, cache);
    ret      = decoder->decode();
    stop();
    return ret;
}

error_t StreamEdge::start() {
    auto ret = ICoFactory::get_instance().start(shared_from_this());

    if (ret == SUCCESS) {
        started = true;
        SingleInstance<StreamEdgeManager>::get_instance().reg(key, shared_from_this());
    }

    return ret;
}

error_t StreamEdge::stop() {
    started = false;

    if (decoder) {
        decoder->stop();
    }

    SingleInstance<StreamEdgeManager>::get_instance().cancel(key);
    return SUCCESS;
}

error_t StreamEdgeEnter::start_edge(const std::string& key, PHostModule& host, PRequestUrl& url) {
    auto proxy           = host->pass_proxy();
    auto n               = proxy.find(":", 0);
    auto proxy_url       = host->pass_url();
    std::string ip;
    int port = 80;

    if (n != std::string::npos) {
        ip   = proxy.substr(0, n);
        port = atoi(proxy.substr(n+1).c_str());
    }

    if (proxy_url.empty()) {
        proxy_url = url->schema + "://" + url->get_host() +  url->get_url();
    }

    PRequestUrl proxy_req;
    auto ret = RequestUrl::from(proxy_url, proxy_req);

    if (ret != SUCCESS) {
        sp_error("proxy url invalid %s %d", proxy_url.c_str(), ret);
        return ret;
    }
    proxy_req->ip   = ip;
    proxy_req->port = port;

    auto cache = StreamCache::create_av_streamcache(key);
    auto edge  = std::make_shared<StreamEdge>(key, proxy_req, ip, port, cache);

    return edge->start();
}

}

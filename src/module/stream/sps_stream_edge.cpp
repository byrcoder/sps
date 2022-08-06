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
#include <utility>

namespace sps {

StreamEdge::StreamEdge(const std::string& key, PRequestUrl url) :
                        key(key) {
    this->key  = key;
    this->url  = std::move(url);
    started    = false;
    cond       = IConditionFactory::get_instance().create_condition();
}

StreamEdge::~StreamEdge() {
    stop();
}

error_t StreamEdge::handler() {
    error_t ret = SUCCESS;
    auto&   avf = SingleInstance<AVDemuxerFactory>::get_instance();
    auto    io  = SingleInstance<sps::UrlProtocol>::get_instance().create(url);

    if (!started) {
        sp_error("stopped");
        goto fail;
    }

    if (!io) {
        sp_error("Fail create url %s", url->get_url());
        goto fail;
    }

    ret = io->open(url, TCP);
    if (ret != SUCCESS || !io->response()->success()) {
        sp_error("Fail Init upstream %s-%s-%s %d %s", url->get_host(), url->get_url(), url->get_params(),
                 ret, io->response() ? io->response()->error().c_str() : "");
        goto fail;
    }

    sp_trace("OK upstream %s-%s-%s", url->get_host(), url->get_url(), url->get_params());

    do {
        auto dec = avf.create(io, url);
        cache    = StreamCache::create_av_streamcache(key);
        cache->set_ctx(dec->get_av_ctx());
        cond->signal();
        decoder  = std::make_shared<StreamDecoder>(dec, cache);
        ret      = decoder->decode();
    } while(false);

fail:
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

    if (cache) {
        StreamCache::release_av_streamcache(key);
    }
    SingleInstance<StreamEdgeManager>::get_instance().cancel(key);

    if (decoder) {
        decoder->stop();
    }

    cond->signal();

    return SUCCESS;
}

error_t StreamEdge::wait(utime_t timeout) {
    cond->wait(timeout);
    return SUCCESS;
}

error_t StreamEdgeEnter::wait_edge(const std::string& key) {
    PStreamEdge* task = SingleInstance<StreamEdgeManager>::get_instance().get(key);
    if (task) {
        return (*task)->wait(10 * 1000 * 1000);
    }

    return ERROR_STREAM_EDGE_NOT_START;
}

error_t StreamEdgeEnter::start_edge(const std::string& key, PHostModule& host, PRequestUrl& url) {
    error_t ret = wait_edge(key);
    if (ret != ERROR_STREAM_EDGE_NOT_START) {
        return ret;
    }

    std::string ip;
    int         port       = url->schema == "rtmp" ? 1935 : (url->schema == "https" ? 443 : 80);
    auto        proxy      = host->pass_proxy();
    auto        proxy_url  = host->pass_url();
    PStreamEdge edge;
    PRequestUrl proxy_req;

    host->get_proxy_info(ip, port);
    if (proxy_url.empty()) {
        proxy_url = url->schema + "://" + proxy +  url->get_url();
    }

    ret = RequestUrl::from(proxy_url, proxy_req);
    if (ret != SUCCESS) {
        sp_error("proxy url invalid %s %d", proxy_url.c_str(), ret);
        goto fail;
    }

    proxy_req->set_ip_port(ip, port);
    edge  = std::make_shared<StreamEdge>(key, proxy_req);
    ret   = edge->start();

fail:
    sp_trace("Starting edge %s:%d, url %s", ip.c_str(), port, proxy_url.c_str());
    return wait_edge(key);
}

}

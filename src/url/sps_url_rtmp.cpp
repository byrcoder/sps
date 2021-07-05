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
// Created by byrcoder on 2021/6/24.
//

#include <sps_url_rtmp.hpp>
#include <sps_log.hpp>

namespace sps {

RtmpUrlProtocol::RtmpUrlProtocol(std::shared_ptr<RtmpHook> hk) {
    this->hk = std::move(hk);
}

error_t RtmpUrlProtocol::open(PRequestUrl &url, Transport tp) {
    error_t        ret   = SUCCESS;
    Transport      p     = (tp == Transport::DEFAULT ? Transport::TCP : tp);
    std::string    ip    = (url->get_ip().empty()) ? (url->get_host()) : (url->get_ip());

    auto socket = SingleInstance<ClientSocketFactory>::get_instance().create_ss(
            p, ip, url->get_port(), url->get_timeout());

    if (!socket) {
        sp_error("failed connect %s:%d, type: %d", ip.c_str(), url->get_port(), p);
        return ERROR_HTTP_SOCKET_CONNECT;
    }

    std::string full_url = url->schema + "://" + url->host + url->url;
    hk = std::make_shared<RtmpHook>(socket);
    ret = hk->client_connect(full_url, url->params, false);

    sp_info("rtmp open %s?%s ret %d", full_url.c_str(), url->params.c_str(), ret);
    return ret;
}

error_t RtmpUrlProtocol::read(void *buf, size_t size, size_t &nread) {
    WrapRtmpPacket packet;
    error_t ret = hk->recv_packet(packet);

    if (ret != SUCCESS) {
        sp_error("recv packet failed ret %d", ret);
        return ret;
    }


    return ret;
}

error_t RtmpUrlProtocol::read(WrapRtmpPacket& packet) {
    error_t ret = hk->recv_packet(packet);

    if (ret != SUCCESS) {
        sp_error("recv packet failed ret %d", ret);
        return ret;
    }
    return ret;
}

error_t RtmpUrlProtocol::write(WrapRtmpPacket& packet) {
    error_t ret = hk->send_packet(packet.packet, false);

    if (ret != SUCCESS) {
        sp_error("send packet failed ret %d", ret);
        return ret;
    }
    return ret;
}

PResponse RtmpUrlProtocol::response() {
    return nullptr;
}

RtmpURLProtocolFactory::RtmpURLProtocolFactory(): IURLProtocolFactory("rtmp", DEFAULT) {
}

PIURLProtocol RtmpURLProtocolFactory::create(PRequestUrl url) {
    return std::make_shared<RtmpUrlProtocol>();
}

}
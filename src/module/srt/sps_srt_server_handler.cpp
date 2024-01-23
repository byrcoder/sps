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
// Created by byrcoder on 2021/8/24.
//

#include <sps_srt_server_handler.hpp>
#include <sps_srt_server.hpp>
#include <sps_sys_st_io_srt.hpp>

#ifdef SRT_ENABLED

namespace sps {

PRequestUrl get_url_by_streamid(const std::string& streamid) {
    auto req = std::make_shared<RequestUrl>();
    if (streamid.size() < 5) {
        sp_error("fail parse srt streamid (%s) size < 5", streamid.c_str());
        return nullptr;
    }

    auto tmp = streamid.substr(4);
    if (tmp.empty()) {
        sp_error("fail parse srt streamid is empty");
        return nullptr;
    }

    char* start = &tmp[0];
    char* p = start;
    char* end = &tmp[0] + tmp.size();

    std::list<RequestParam> params;

    RequestParam param;
    while (p != end) {
        if (*p == ',') {
            param.value = std::string(start, p-start);
            params.push_back(param);
            start = p+1;

            param.key = param.value = "";  // clear value
        } else if (*p == '=') {
            param.key = std::string(start, p-start);
            start = p+1;
        }
        ++p;
    }

    std::string url = "";
    std::string mode = "";
    for(auto& p : params) {
        if (p.key == "h") {
            url = p.value;
        } else if (p.key == "m") {
            mode = p.value;
        }
    }

    if (url.substr(0, 5) != "srt://") {
        url = "srt://" + url;
    }

    auto ret = RequestUrl::from(url, req);

    if (ret != SUCCESS) {
        sp_error("fail parse srt streamid %s, url %s, mode %s.",
                 streamid.c_str(), url.c_str(), mode.c_str());
        return nullptr;
    }

    if (mode == "publish") {
        req->method = "publish";
    } else if (mode == "request") {
        req->method = "request";
    } else {
        req->method = "publish";
    }

    sp_info("success parse srt streamid %s, url %s, mode %s",
             streamid.c_str(), url.c_str(), mode.c_str());

    return req;
}

SrtPrepareHandler::SrtPrepareHandler()
        : IPhaseHandler("srt-streamid-http_server") {
}

// TODO: impl stream parse
error_t SrtPrepareHandler::handler(IConnection &c) {
    auto   srt_io = dynamic_cast<StSrtSocket*>(c.socket->get_io().get());

    if (srt_io == nullptr) {
        sp_error("not srt io");
        return ERROR_IO_EOF;
    }

    // streamid=#!::h=live/livestream,m=publish
    auto streamid = srt_io->get_streamid();
    auto req      = get_url_by_streamid(streamid);
    if (!req) {
        sp_error("fail parse streamid");
        return ERROR_SRT_STREAM_ID;
    }
    auto ctx = std::make_shared<SrtContext>(req);
    c.set_context(ctx);

    return SUCCESS;
}

}

#endif

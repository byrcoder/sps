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

#include <sps_avformat_dec.hpp>

#include <algorithm>

#include <sps_log.hpp>

namespace sps {

IAVInputFormat::IAVInputFormat(const char *name, const char *ext) {
    this->name = name;
    this->ext  = ext;
}

bool IAVInputFormat::match(const char *e) const {
    int n = strlen(ext);
    int m = strlen(ext);
    return  memcmp(ext, e, std::max(n, m));
}

PIAVDemuxer IAVInputFormat::create2(PIReader p) {
    auto remuxer = _create(p);
    if (remuxer) {
        remuxer->fmt = this;
    }
    return remuxer;
}

PIAVDemuxer AVDemuxerFactory::probe(PIReader p, PRequestUrl& url) {
    return nullptr;
}

PIAVDemuxer AVDemuxerFactory::create(PIReader p, PRequestUrl& url) {
    auto& fmts = refs();

    for (auto& f : fmts) {
        if (f->match(url->get_ext())) {
            return f->create2(std::move(p));
        }
    }
    return probe(std::move(p), url);
}

PIAVDemuxer AVDemuxerFactory::create(PIReader p, const std::string &url) {
    PRequestUrl purl = std::make_shared<RequestUrl>();

    if (purl->parse_url(url) != SUCCESS) {
        sp_error("Invalid url %s.", url.c_str());
        return nullptr;
    }
    return create(std::move(p), purl);
}

}

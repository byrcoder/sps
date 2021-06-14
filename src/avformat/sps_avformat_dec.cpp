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
#include <sps_log.hpp>

namespace sps {

IAVInputFormat::IAVInputFormat(const char *name, const char *ext) {
    this->name = name;
    this->ext_name = ext;
}

bool IAVInputFormat::match(const char *ext) const {
    int n = strlen(ext);
    int m = strlen(ext_name);
    return  memcmp(ext, ext_name, std::max(n, m));
}

error_t AvDemuxerFactory::init() {
    return SUCCESS;
}

PIAvDemuxer AvDemuxerFactory::probe(PIURLProtocol& p, PRequestUrl& url) {
    return nullptr;
}

PIAvDemuxer AvDemuxerFactory::create(PIURLProtocol& p, PRequestUrl& url) {
    auto& fmts = refs();

    for (auto& f : fmts) {
        if (f->match(url->get_ext())) {
            return f->create(p);
        }
    }

    return probe(p, url);
}

PIAvDemuxer AvDemuxerFactory::create(PIURLProtocol& p, const std::string &url) {
    PRequestUrl purl = std::make_shared<RequestUrl>();

    if (purl->parse_url(url) != SUCCESS) {
        sp_error("Invalid url %s.", url.c_str());
        return nullptr;
    }

    return create(p, purl);
}

}
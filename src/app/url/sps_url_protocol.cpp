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

#include <app/url/sps_url_protocol.hpp>
#include <log/sps_log.hpp>

namespace sps {

error_t IURLProtocol::open(const std::string url, Transport p) {
    auto req = std::make_shared<RequestUrl>();
    error_t ret = SUCCESS;
    if ((ret = req->parse_url(url)) != SUCCESS) {
        return ret;
    }

    return open(req, p);
}

IUrlProtocolFactory::IUrlProtocolFactory(const char *schema, Transport t) {
    this->schema = schema;
    this->t     = t;
}

bool IUrlProtocolFactory::match(PRequestUrl url) {
    return url->schema == this->schema;
}

PIURLProtocol UrlProtocol::create(PRequestUrl url) {
    auto& ps = refs();
    for (auto& p : ps) {
        if (p->match(url)) {
            sp_debug("create by %s", p->schema);
            return p->create(url);
        }
    }
    return nullptr;
}

PIURLProtocol UrlProtocol::create(const std::string &url) {
    auto req    = std::make_shared<RequestUrl>();
    error_t ret = RequestUrl::from(url, req);

    if (ret != SUCCESS) {
        return nullptr;
    }

    return create(req);
}

}
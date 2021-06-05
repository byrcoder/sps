#include <app/url/protocol.hpp>
#include <log/logger.hpp>

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
            sp_info("create by :%s", p->schema);
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
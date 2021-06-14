#include <app/stream/sps_stream_dec.hpp>

#include <app/stream/sps_stream_flvdec.hpp>
#include <log/sps_log.hpp>

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
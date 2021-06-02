#include <app/upstream/upstream.hpp>

#include <app/upstream/http_upstream.hpp>

namespace sps {

IUpstream::IUpstream(PICacheStream cs) {
    this->cs = cs;
}

IUpstream::~IUpstream() {
    abort_request();
    container->delete_upstream(shared_from_this());
}

void IUpstream::abort_request() {
    cs->close();
}

error_t IUpstream::open_url(const std::string &url) {
    auto    req = std::make_shared<RequestUrl>();
    error_t ret = req->parse_url(url);

    if (ret != SUCCESS) {
        return ret;
    }

    return open_url(req);
}

error_t IUpstream::handler() {
    PIBuffer header;
    error_t ret = protocol->read_header(header);
    if (ret != SUCCESS) {
        sp_error("Failed read head:%d", ret);
        return ret;
    }

    cs->put(header);
    do {
        PIBuffer body;
        if ((ret = protocol->read_message(body)) != SUCCESS) {
            break;
        }
        cs->put(body);
    } while(true);

    if (ret != PROTOCOL_EOF) {
        sp_error("Failed read body:%d", ret);
        return ret;
    }

    PIBuffer tail;
    if ((ret = protocol->read_tail(tail)) != SUCCESS) {
        sp_error("Failed read tail:%d", ret);
        return ret;
    }
    cs->put(tail);

    return ret;
}

void IUpstream::on_stop() {

}

PICacheStream IUpstream::get_cs() {
    return cs;
}

PIUpstream UpstreamFactory::create_upstream(PICacheStream cs, PRequestUrl url) {
    if (url->schema == "http") {
        return std::make_shared<HttpUpstream>(cs);
    }
    return nullptr;
}

}

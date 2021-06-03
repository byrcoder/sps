#include <app/upstream/http_upstream.hpp>

#include <net/socket.hpp>

namespace sps {

HttpUpstream::HttpUpstream(PICacheStream cs, Protocol p) : IUpstream(std::move(cs), p) {

}

error_t HttpUpstream::open_url(PRequestUrl req, utime_t tm) {
    auto skt = SingleInstance<ClientSocketFactory>::get_instance().create_ss(
            Transport::TCP, req->get_host(), req->get_port(), tm);

    if (skt == nullptr) {
        return ERROR_ST_OPEN_SOCKET;
    }

    IUpstream::protocol  = SingleInstance<ProtocolFactory>::get_instance().create_protocol(p, skt);
    return protocol == nullptr ? ERROR_PROTOCOL_NOT_DEFINE : SUCCESS;
}

void HttpUpstream::abort_request() {

}

}

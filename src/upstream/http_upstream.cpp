#include <upstream/http_upstream.hpp>

namespace sps {

HttpUpstream::HttpUpstream(PICacheStream cs) : IUpstream(std::move(cs)) {

}

error_t HttpUpstream::open_url(PRequestUrl req) {
    return SUCCESS;
}

}

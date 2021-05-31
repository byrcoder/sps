#include <cache/cache.hpp>

namespace sps {

bool ICacheStream::eof() {
    return is_eof;
}

void ICacheStream::close() {
    is_eof = true;
}

error_t CacheStream::put(PIBuffer pb) {
    if (!pb)  return SUCCESS;

    this->pbs.push_back(pb);
    this->publish(pb);
    return SUCCESS;
}

int CacheStream::dump(std::list<PIBuffer> &vpb, bool remove) {
    vpb = pbs;
    if (remove) {
        pbs.clear();
    }
    return (int) pbs.size();
}

int CacheStream::size() {
    int sz = 0;

    for (auto& pb : pbs) {
        sz += pb->size();
    }

    return sz;
}

int CacheStream::do_event(PIBuffer& o) {
    return put(o);
}

PICacheStream InfiniteCache::get(const std::string &name) {
    auto it = css.find(name);
    return (it == css.end()) ? nullptr : it->second;
}

error_t InfiniteCache::put(const std::string &name, PICacheStream cs) {
    css[name] = cs;
    return SUCCESS;
}

}

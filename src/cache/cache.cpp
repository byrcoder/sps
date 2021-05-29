#include <cache/cache.hpp>

namespace sps {

error_t CacheStream::put(PIBuffer pb) {
    this->pbs.push_back(std::move(pb));
}

int CacheStream::dump(std::list<PIBuffer> &vpb) {
    vpb = pbs;
    return (int) pbs.size();
}

int CacheStream::size() {
    int sz = 0;

    for (auto& pb : pbs) {
        sz += pb->size();
    }

    return sz;
}

PICacheStream InfiniteCache::get(const std::string &name) {
    auto it = css.find(name);
    return it != css.end() ? it->second : nullptr;
}

error_t InfiniteCache::put(const std::string &name, PICacheStream cs) {
    css[name] = cs;
}

}

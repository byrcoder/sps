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

#include <cache/sps_cache.hpp>

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

error_t InfiniteCache::erase(const std::string &name) {
    css.erase(name);
    return SUCCESS;
}

PICacheStream InfiniteCache::operator[](const std::string &name) {
    auto it = css.find(name);
    return it == css.end() ? nullptr : it->second;
}

}

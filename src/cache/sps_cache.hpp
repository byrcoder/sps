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

#ifndef SPS_CACHE_HPP
#define SPS_CACHE_HPP

#include <list>
#include <map>
#include <memory>
#include <string>

#include <sps_sync.hpp>
#include <sps_typedef.hpp>

namespace sps {

/*
 * recv-upstream cache_stream (publisher)----> visitor cache_stream (subscriber) --> wakeup -> send
 *                                       \---> visitor cache_stream (subscriber) --> wakeup -> send
 *
 */

// 流媒体缓存器，包含flv/rtmp 的gop cache，以及文件类型的cache

template<class T>
class ICacheStream: public Subscriber<T>, public Publisher<T> {
 public:
    typedef std::shared_ptr<ICacheStream<T>> PICacheStream;

 public:
    virtual ~ICacheStream() = default;

 public:
    virtual error_t put(T pb) = 0;
    virtual int dump(std::list<T>& vpb, bool remove = false) = 0;
    virtual int size() = 0;
    virtual bool empty() = 0;
    virtual bool eof() { return is_eof; }
    virtual void close() { is_eof = true; }
    virtual void* get_ctx() { return nullptr; }
    virtual void  set_ctx(void* ctx) {        }

 public:
    error_t do_event(T& o) override  { return SUCCESS; }

 private:
    bool is_eof = false;
};

template<class T>
class CacheStream : public ICacheStream<T> {
 public:
    error_t put(T pb) override {
        if (!pb)  return SUCCESS;

        this->pbs.push_back(pb);
        this->publish(pb);
        return SUCCESS;
    }

    error_t copy_from(ICacheStream<T>& cache) {
        cache.dump(pbs, false);
        return SUCCESS;
    }

    int dump(std::list<T>& vpb, bool remove = false) override {
        vpb.insert(vpb.end(), pbs.begin(), pbs.end());
        int n = pbs.size();

        if (remove) {
            pbs.clear();
        }
        return n;
    }

    int size() override {
        return pbs.size();
    }

    bool empty() override {
        return pbs.empty();
    }

    void* get_ctx() override {
        return ctx;
    }

    void set_ctx(void* ctx) override {
        this->ctx = ctx;
    }

 public:
    error_t do_event(T& o) override {
        return put(o);
    }

 protected:
    std::list<T> pbs;
    void* ctx = nullptr;
};

template<class T>
class ICache {
    typedef typename ICacheStream<T>::PICacheStream PICacheStream;
 public:
    typedef std::shared_ptr<ICache<T>> PICache;

 public:
    virtual ~ICache() = default;

 public:
    virtual PICacheStream get(const std::string& name) = 0;
    virtual error_t       put(const std::string& name, PICacheStream cs) = 0;
    virtual error_t       erase(const std::string& name) = 0;
    virtual PICacheStream operator[](const std::string& name) = 0;
};

template<class T>
class InfiniteCache : public ICache<T> {
 public:
    typedef typename ICacheStream<T>::PICacheStream PICacheStream;

 public:
    PICacheStream get(const std::string& name) override {
        auto it = css.find(name);
        return (it == css.end()) ? nullptr : it->second;
    }

    error_t put(const std::string& name, PICacheStream cs) override {
        css[name] = cs;
        return SUCCESS;
    }

    error_t erase(const std::string& name) override {
        css.erase(name);
        return SUCCESS;
    }

    PICacheStream operator[](const std::string& name) override {
        auto it = css.find(name);
        return it == css.end() ? nullptr : it->second;
    }

 private:
    std::map<std::string, PICacheStream> css;
};

}  // namespace sps

#endif  // SPS_CACHE_HPP

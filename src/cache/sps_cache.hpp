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

#include <cache/sps_cache_buffer.hpp>
#include <sync/sps_sync.hpp>
#include <sps_typedef.hpp>

namespace sps {

class ICacheStream;
typedef std::shared_ptr<ICacheStream> PICacheStream;
/*
 * recv-upstream cache_stream (publisher)----> visitor cache_stream (subscriber) --> wakeup -> send
 *                                       \---> visitor cache_stream (subscriber) --> wakeup -> send
 *
 */

// 流媒体缓存器，包含flv/rtmp 的gop cache，以及文件类型的cache
class ICacheStream : public Subscriber<PIBuffer>, public Publisher<PIBuffer> {
 public:
    virtual ~ICacheStream() = default;

 public:
    virtual error_t put(PIBuffer pb) = 0;
    virtual int dump(std::list<PIBuffer>& vpb, bool remove = false) = 0;
    virtual int size() = 0;
    virtual bool eof();
    virtual void close();

 public:
    int do_event(PIBuffer& o) override  { return SUCCESS; }

 private:
    bool is_eof = false;
};

class CacheStream : public ICacheStream {
 public:
    error_t put(PIBuffer pb) override;
    int dump(std::list<PIBuffer>& vpb, bool remove = false) override;
    int size() override ;

 public:
    int do_event(PIBuffer& o) override;

 private:
    std::list<PIBuffer> pbs;
};

class ICache {
 public:
    virtual ~ICache() = default;

 public:
    virtual PICacheStream get(const std::string& name) = 0;
    virtual error_t       put(const std::string& name, PICacheStream cs) = 0;
};

typedef std::shared_ptr<ICache> PICache;

class InfiniteCache : public ICache {
 public:
    PICacheStream get(const std::string& name) override;
    error_t       put(const std::string& name, PICacheStream cs) override;

 private:
    std::map<std::string, PICacheStream> css;
};

}

#endif  // SPS_CACHE_HPP

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

//
// Created by byrcoder on 2021/6/15.
//

#ifndef SPS_STREAM_CACHE_HPP
#define SPS_STREAM_CACHE_HPP

#include <sps_host_phase_handler.hpp>

#include <sps_avformat_cache.hpp>
#include <sps_cache.hpp>

namespace sps {

class StreamCache : public InfiniteCache<PAVPacket>,
                    public SingleInstance<StreamCache> {
 public:
    typedef typename ICacheStream<PAVPacket>::PICacheStream PICacheStream;

 public:
    static PICacheStream get_streamcache(const std::string& url) {
        return get_instance().get(url);
    }

    static PICacheStream create_av_streamcache(const std::string& url) {
        auto cache = std::make_shared<AVGopCacheStream>();
        get_instance().put(url, cache);
        return cache;
    }

    static void release_av_streamcache(const std::string& url) {
        get_instance().erase(url);
    }

    static PAVDumpCacheStream get_client_streamcache(const std::string& url, utime_t timeout) {
        auto cache = get_streamcache(url);
        if (!cache) {
            return nullptr;
        }
        auto client_cache = std::make_shared<AVDumpCacheStream>(timeout);
        cache->sub(client_cache);
        auto& obj_cache = *cache.get();
        client_cache->copy_from(obj_cache);
        return client_cache;
    }
};

}  // namespace sps

#endif  // SPS_STREAM_CACHE_HPP

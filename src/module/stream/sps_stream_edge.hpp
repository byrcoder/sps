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

#ifndef SPS_STREAM_EDGE_HPP
#define SPS_STREAM_EDGE_HPP

#include <sps_typedef.hpp>
#include <sps_stream.hpp>

namespace sps {

class StreamEdge;
typedef std::shared_ptr<StreamEdge> PStreamEdge;
class StreamEdgeManager : public KeyRegisters<std::string, PStreamEdge> {
};

class StreamEdge : public ICoHandler, public std::enable_shared_from_this<StreamEdge> {
 public:
    StreamEdge(const std::string& key, PRequestUrl url);
    ~StreamEdge();

 public:
    error_t handler() override;

 public:
    error_t start();
    error_t stop();
    error_t wait(utime_t timeout);

 private:
    bool           started;
    std::string    key;  // cache key
    PRequestUrl    url;
    PStreamDecoder decoder;
    StreamCache::PICacheStream cache;
    PCondition     cond;
};

class StreamEdgeEnter {
 public:
    static error_t start_edge(const std::string& key, PHostModule& host, PRequestUrl& url);
    static error_t wait_edge(const std::string& key);
};

}  // namespace sps

#endif  // SPS_STREAM_EDGE_HPP

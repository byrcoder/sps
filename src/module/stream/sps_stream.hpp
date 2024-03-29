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

#ifndef SPS_STREAM_HPP
#define SPS_STREAM_HPP

#include <sps_avformat_dec.hpp>
#include <sps_avformat_enc.hpp>
#include <sps_stream_cache.hpp>

namespace sps {

class StreamDecoder {
 public:
    StreamDecoder(PIAVDemuxer demuxer, StreamCache::PICacheStream cache);

 public:
    error_t decode();
    error_t stop();

 private:
    bool running;
    PIAVDemuxer dec;
    StreamCache::PICacheStream cache;
};
typedef std::shared_ptr<StreamDecoder> PStreamDecoder;

class StreamEncoder {
 public:
    StreamEncoder(PIAVMuxer muxer, PAVDumpCacheStream cache, bool wrote_header = true);

 public:
    error_t encode();

 private:
    PIAVMuxer enc;
    PAVDumpCacheStream cache;
    bool      wrote_header;
};

}  // namespace sps

#endif  // SPS_STREAM_HPP

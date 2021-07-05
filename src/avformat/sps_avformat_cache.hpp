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

#ifndef SPS_AVFORMAT_CACHE_HPP
#define SPS_AVFORMAT_CACHE_HPP

#include <list>

#include <sps_cache.hpp>
#include <sps_avformat_packet.hpp>

namespace sps {

// publisher, save gop cache
class AVGopCacheStream : public CacheStream<PAVPacket> {
 public:
    error_t put(PAVPacket pb) override;

    int dump(std::list<PAVPacket>& vpb, bool /** remove **/) override;

 private:
    PAVPacket video_sequence_header;
    PAVPacket audio_sequence_header;
    PAVPacket script;
};

// subscriber dump from gop cache and subscribe AVGopCacheStream
class AVDumpCacheStream : public CacheStream<PAVPacket> {
 public:
    explicit AVDumpCacheStream(utime_t recv_timeout);

 public:
    int dump(std::list<PAVPacket>& vpb, bool /** remove **/) override;

 private:
    utime_t recv_timeout;
};

}  // namespace sps

#endif  // SPS_AVFORMAT_CACHE_HPP

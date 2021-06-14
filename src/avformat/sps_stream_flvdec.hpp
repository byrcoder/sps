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

#ifndef SPS_STREAM_FLVDEC_HPP
#define SPS_STREAM_FLVDEC_HPP

#include <app/stream/sps_stream_msg.hpp>
#include <avformat/sps_stream_dec.hpp>
#include <net/sps_net_io.hpp>
#include <vector>

namespace sps {

class FlvDecoder : public IAvDemuxer {
 public:
    FlvDecoder(PIReader rd);

 public:
    error_t read_header(PIBuffer& buffer)  override;
    error_t read_message(PIBuffer& buffer) override;
    error_t read_tail(PIBuffer& buffer)    override;
    error_t probe(PIBuffer& buffer)        override;

 private:
    std::vector<char> buf;
    PIReader rd;

 public:
    static const int max_len = 1 * 1024 * 1024;
};

class FlvInputFormat : public IAVInputFormat {
 public:
    FlvInputFormat();

 public:
    PIAvDemuxer create(PIURLProtocol s) override;

};

}

#endif // SPS_STREAM_FLVDEC_HPP

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
// Created by byrcoder on 2021/7/23.
//

#include <sps_avformat_tsdec.hpp>

#include <utility>

#include <sps_io_bytes.hpp>

#include <sps_log.hpp>

namespace sps {

TsDemuxer::TsDemuxer(PIReader rd) {
    this->io = std::move(rd);
}

error_t TsDemuxer::read_header(PSpsAVPacket & buffer) {
    return SUCCESS;
}

error_t TsDemuxer::read_packet(PSpsAVPacket& buffer) {
    return ERROR_STREAM_NOT_IMPL;
}

error_t TsDemuxer::read_tail(PSpsAVPacket& buffer) {
    return SUCCESS;
}

error_t TsDemuxer::probe(PSpsAVPacket& buffer) {
    return ERROR_STREAM_NOT_IMPL;
}

}  // namespace sps

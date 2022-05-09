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
// Created by byrcoder on 2021/6/16.
//

#include <sps_avformat_dec.hpp>
#include <sps_avformat_enc.hpp>

#include <sps_avformat_flvdec.hpp>
#include <sps_avformat_flvenc.hpp>

#include <sps_avformat_rtmpdec.hpp>
#include <sps_avformat_rtmpenc.hpp>

#include <sps_avformat_tsdec.hpp>

#include <sps_avformat_ffmpeg_dec.hpp>
#include <sps_avformat_ffmpeg_enc.hpp>

namespace sps {

#define AVINPUTFORMAT_INSTANCE(NAME) (std::make_shared<NAME##AVInputFormat>())

PIAVInputFormat av_input_formats[] = {
        AVINPUTFORMAT_INSTANCE(FFmpeg),
        AVINPUTFORMAT_INSTANCE(Flv),
        AVINPUTFORMAT_INSTANCE(Rtmp),
        AVINPUTFORMAT_INSTANCE(Ts),
        nullptr,
};


#define AVOUTPUTFORMAT_INSTANCE(NAME) (std::make_shared<NAME##AVOutputFormat>())

PIAVOutputFormat av_output_formats[] = {
        AVOUTPUTFORMAT_INSTANCE(FFmpeg),
        AVOUTPUTFORMAT_INSTANCE(Flv),
        AVOUTPUTFORMAT_INSTANCE(Rtmp),
        nullptr,
};

}  // namespace sps

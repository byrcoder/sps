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
// reference ffmpeg avformat flv.h
// Created by byrcoder on 2021/6/17.
//

#ifndef SPS_AVFORMAT_FLV_HPP
#define SPS_AVFORMAT_FLV_HPP

enum {
    FLV_HEADER_FLAG_HASVIDEO = 1,
    FLV_HEADER_FLAG_HASAUDIO = 4,
};

enum FlvTagType {
    FLV_TAG_TYPE_AUDIO = 0x08,
    FLV_TAG_TYPE_VIDEO = 0x09,
    FLV_TAG_TYPE_META  = 0x12,
};

enum {
    FLV_STREAM_TYPE_VIDEO,
    FLV_STREAM_TYPE_AUDIO,
    FLV_STREAM_TYPE_SUBTITLE,
    FLV_STREAM_TYPE_DATA,
    FLV_STREAM_TYPE_NB,
};

#endif  // SPS_AVFORMAT_FLV_HPP

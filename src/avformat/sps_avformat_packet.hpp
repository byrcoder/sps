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
// Created by byrcoder on 2021/6/17.
//
#ifndef SPS_AVFORMAT_PACKET_HPP
#define SPS_AVFORMAT_PACKET_HPP

#include <memory>

#include <sps_cache_buffer.hpp>

namespace sps {

#define FLV_HEAD_TAG_SIZE 20

// The packet contains a keyframe
const int AV_PKT_FLAG_KEY       =   0x0001;
// The packet contains a sequence header
const int AV_PKT_FLAG_SEQUENCE  =   0x0002;

enum SpsMessageType {
    AV_MESSAGE_HEADER,
    AV_MESSAGE_DATA,
    AV_MESSAGE_TAIL,
    AV_MESSAGE_NB,
};

enum SpsAVStreamType {
    AV_STREAM_TYPE_VIDEO,
    AV_STREAM_TYPE_AUDIO,
    AV_STREAM_TYPE_SUBTITLE,
    AV_STREAM_TYPE_DATA,
    AV_STREAM_TYPE_NB,
};

enum SpsVideoPktType {
    AV_VIDEO_TYPE_SEQUENCE_HEADER = 0,
};

enum SpsAudioPktType {
    AV_AUDIO_TYPE_SEQUENCE_HEADER = 0,
};

struct SpsAVPacketType {
    int pkt_type;
};

enum SpsAudioCodec {
    AAC = 10,
    MP3 = 14,
};

enum SpsVideoCodec {
    H264 = 7,   // AVC
    H265 = 12,  // H265
};


class SpsAVPacket;
typedef std::shared_ptr<SpsAVPacket> PSpsAVPacket;
// ffmpeg
class SpsAVPacket : public CharBuffer {
 public:
    static PSpsAVPacket create(
            SpsMessageType msg_type,
            SpsAVStreamType stream_type,
            SpsAVPacketType pkt_type,
            uint8_t* buf,
            int len,
            int64_t dts,
            int64_t pts,
            int flags = 0,
            int codecid = 0,
            int64_t duration = 0);

 public:
    // head_len for rtmp
    SpsAVPacket(uint8_t* buf, int len, int head_len = 0);

 public:
    bool is_video() const;
    bool is_audio() const;
    bool is_script() const;
    bool is_keyframe() const;
    bool is_video_sequence_header() const;
    bool is_audio_sequence_header();

 public:
    /**
     * avformat header/data/tail
     */
    SpsMessageType msg_type   = AV_MESSAGE_NB;

    /**
     * video/audio/metadata/
     */
    SpsAVStreamType stream_type  = AV_STREAM_TYPE_NB;

    SpsAVPacketType     pkt_type = {};

    /**
     * Presentation timestamp in AVStream->time_base units; the time at which
     * the decompressed packet will be presented to the user.
     * Can be AV_NOPTS_VALUE if it is not stored in the file.
     * pts MUST be larger or equal to dts as presentation cannot happen before
     * decompression, unless one wants to view hex dumps. Some formats misuse
     * the terms dts and pts/cts to mean something different. Such timestamps
     * must be converted to true pts/dts before they are stored in AVPacket.
     */
    int64_t pts  =  -1;

    /**
     * Decompression timestamp in AVStream->time_base units; the time at which
     * the packet is decompressed.
     * Can be AV_NOPTS_VALUE if it is not stored in the file.
     */
    int64_t dts  = -1;
    /**
     * A combination of AV_PKT_FLAG values
     */
    int     flags = 0;

    /**
     * Duration of this packet in AVStream->time_base units, 0 if unknown.
     * Equals next_pts - this_pts in presentation order.
     */
    int64_t duration = 0;

    int     codecid = 0;

    int     number  = 0;

 public:
    void debug();
};
typedef std::shared_ptr<SpsAVPacket> PAVPacket;

}  // namespace sps

#endif  // SPS_AVFORMAT_PACKET_HPP

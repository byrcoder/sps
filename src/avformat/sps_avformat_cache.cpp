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
// Created by byrcoder on 2021/7/4.
//

#include <sps_avformat_cache.hpp>

namespace sps {

error_t AVGopCacheStream::put(PAVPacket pb) {
    if (!pb) return SUCCESS;

    pb->number = number++;
    if (pb->is_audio_sequence_header()) {
        sp_info("new audio header dts %lld", pb->dts);
        audio_sequence_header = pb;
    } else if (pb->is_video_sequence_header()) {
        sp_info("new video header dts %lld", pb->dts);
        video_sequence_header = pb;
    } else if (pb->is_script()) {
        sp_info("new script dts %lld", pb->dts);
        script = pb;
    }  else if (pb->is_keyframe()) {
        pbs.clear();  // new keyframe
        pbs.push_back(pb);
        sp_info("%d. new keyframe dts %lld", pb->number, pb->dts);
    } else {
        if (!pbs.empty()) {
            pbs.push_back(pb);  // in case no keyframe
        }
    }

    this->publish(pb);
    return SUCCESS;
}

int AVGopCacheStream::dump(std::list<PAVPacket>& vpb, bool) {
    if (script) {
        vpb.push_back(script);
    }

    if (audio_sequence_header) {
        vpb.push_back(audio_sequence_header);
    }

    if (video_sequence_header) {
        vpb.push_back(video_sequence_header);
    }

    return CacheStream::dump(vpb, false);
}

error_t MepegtsCacheStream::put(PAVPacket pb) {
    if (pb->is_pat()) {
        pat_header = pb;
    } else if (pb->is_pmt()) {
        pmt_header = pb;
    } else {
        if (pb->is_payload_unit_start_indicator()) {
            pbs.clear();
        }
        pbs.push_back(pb);
    }
    this->publish(pb);
    return SUCCESS;
}

int MepegtsCacheStream::dump(std::list<PAVPacket> &vpb, bool) {
    if (pat_header) {
        vpb.push_back(pat_header);
    }

    if (pmt_header) {
        vpb.push_back(pmt_header);
    }

    return CacheStream::dump(vpb, false);
}

AVDumpCacheStream::AVDumpCacheStream(utime_t recv_timeout) {
    this->recv_timeout = recv_timeout;
}

int AVDumpCacheStream::dump(std::list<PAVPacket> &vpb, bool) {
    if (size() == 0) {
        wait(recv_timeout);
    }

    if (size() == 0) {
        return 0;
    }
    return CacheStream::dump(vpb, true);
}

}  // namespace sps

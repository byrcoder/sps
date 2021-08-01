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

#ifndef SPS_AVCODEC_H264_PARSER_HPP
#define SPS_AVCODEC_H264_PARSER_HPP

#include <sps_typedef.hpp>

#include <memory>
#include <vector>

#include <sps_avformat_packet.hpp>

/*
 * Table 7-1 – NAL unit type codes, syntax element categories, and NAL unit type classes in
 * T-REC-H.264-201704
 */
enum {
    H264_NAL_UNSPECIFIED     = 0,
    H264_NAL_SLICE           = 1,
    H264_NAL_DPA             = 2,
    H264_NAL_DPB             = 3,
    H264_NAL_DPC             = 4,
    H264_NAL_IDR_SLICE       = 5,
    H264_NAL_SEI             = 6,
    H264_NAL_SPS             = 7,
    H264_NAL_PPS             = 8,
    H264_NAL_AUD             = 9,
    H264_NAL_END_SEQUENCE    = 10,
    H264_NAL_END_STREAM      = 11,
    H264_NAL_FILLER_DATA     = 12,
    H264_NAL_SPS_EXT         = 13,
    H264_NAL_PREFIX          = 14,
    H264_NAL_SUB_SPS         = 15,
    H264_NAL_DPS             = 16,
    H264_NAL_RESERVED17      = 17,
    H264_NAL_RESERVED18      = 18,
    H264_NAL_AUXILIARY_SLICE = 19,
    H264_NAL_EXTEN_SLICE     = 20,
    H264_NAL_DEPTH_EXTEN_SLICE = 21,
    H264_NAL_RESERVED22      = 22,
    H264_NAL_RESERVED23      = 23,
    H264_NAL_UNSPECIFIED24   = 24,
    H264_NAL_UNSPECIFIED25   = 25,
    H264_NAL_UNSPECIFIED26   = 26,
    H264_NAL_UNSPECIFIED27   = 27,
    H264_NAL_UNSPECIFIED28   = 28,
    H264_NAL_UNSPECIFIED29   = 29,
    H264_NAL_UNSPECIFIED30   = 30,
    H264_NAL_UNSPECIFIED31   = 31,
};

namespace sps {

class H264NAL {
 public:
    H264NAL() = default;
    ~H264NAL() = default;

 public:
    error_t is_pps();
    error_t is_sps();
    error_t is_idr();
    error_t is_aud();

 public:
    error_t parse_nal(uint8_t* buf, size_t sz);

 public:
    struct {
        uint8_t forbidden_bit : 1;
        uint8_t nal_reference_bit : 2;
        uint8_t nal_unit_type : 5;
    } nal_header;

    uint8_t* nalu_buf   = nullptr;
    size_t   nalu_size  = 0;
};

class NALUContext {
 public:
    NALUContext(int64_t dts = 0, int64_t pts = 0);

 public:
    error_t append(uint8_t* nal, size_t sz);

 public:
    std::vector<H264NAL> nalu_list;

 public:
    int64_t dts;
    int64_t pts;
};
typedef std::shared_ptr<NALUContext> PNALUContext;

enum {
    H264_FORMAT_UNKNOWN,
    H264_FORMAT_AVC,
    H264_FORMAT_ANNEX_B,
};

uint8_t* find_start_code(uint8_t* buf, size_t sz);

class NALUParser {
 public:
    NALUParser(bool is_avc = false, int nalu_length_size_minus_one = -1);

 public:
    // if not sure is_seq_header set false
    // avc must make sure whether is_seq_header
    // annexb can ignore is_seq_header
    error_t decode(uint8_t* buf, size_t sz,
                   NALUContext* ctx,
                   bool is_seq_header);
 private:
    error_t decode_header(uint8_t* buf, size_t sz,
                          NALUContext* ctx);
    error_t decode_data(uint8_t* buf, size_t sz,
                        NALUContext* ctx);

 public:
    error_t encode_avc(NALUContext* ctx, std::list<PAVPacket>& pkts);
    error_t encode_annexb(NALUContext* ctx, PAVPacket& pkt);

 private:
    bool is_avc;  // default
    int8_t nalu_length_size_minus_one;
};

}  // namespace sps

#endif  // SPS_AVCODEC_H264_PARSER_HPP

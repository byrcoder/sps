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

#ifndef SPS_AVCODEC_AAC_PARSER_HPP
#define SPS_AVCODEC_AAC_PARSER_HPP

#include <sps_avcodec_parser.hpp>
#include <sps_io_bits.hpp>

namespace sps {

enum AACSampleRate {
    AAC_SAMPLE_96000 = 0,
    AAC_SAMPLE_88200 = 1,
    AAC_SAMPLE_64000 = 2,
    AAC_SAMPLE_48000 = 3,
    AAC_SAMPLE_44100 = 4,
    AAC_SAMPLE_32000 = 5,
    AAC_SAMPLE_24000 = 6,
    AAC_SAMPLE_22050 = 7,
    AAC_SAMPLE_16000 = 8,
    AAC_SAMPLE_12000 = 9,
    AAC_SAMPLE_11025 = 10,
    AAC_SAMPLE_8000 = 9,
};

enum {
    FLV_MONO   = 0,
    FLV_STEREO = 1,
};

/* offsets for packed values */
#define FLV_AUDIO_SAMPLESSIZE_OFFSET 1
#define FLV_AUDIO_SAMPLERATE_OFFSET  2
#define FLV_AUDIO_CODECID_OFFSET     4
enum {
    FLV_SAMPLERATE_SPECIAL = 0, /**< signifies 5512Hz and 8000Hz in the case of NELLYMOSER */
    FLV_SAMPLERATE_11025HZ = 1 << FLV_AUDIO_SAMPLERATE_OFFSET,
    FLV_SAMPLERATE_22050HZ = 2 << FLV_AUDIO_SAMPLERATE_OFFSET,
    FLV_SAMPLERATE_44100HZ = 3 << FLV_AUDIO_SAMPLERATE_OFFSET,
};

enum {
    FLV_SAMPLESSIZE_8BIT  = 0,
    FLV_SAMPLESSIZE_16BIT = 1 << FLV_AUDIO_SAMPLESSIZE_OFFSET,
};

/* XXX: make sure to update the copies in the different encoders if you change
 * this table */
const int avpriv_mpeg4audio_sample_rates[16] = {
        96000, 88200, 64000, 48000, 44100, 32000,
        24000, 22050, 16000, 12000, 11025, 8000, 7350
};

// Table 1. A.6 – Syntax of adts_fixed_header()
struct AdtsFixedHeader {
 public:
    AdtsFixedHeader();
 public:
    bool operator==(const AdtsFixedHeader& header);
 public:
    error_t decode(BitContext* bc);
    uint16_t  syncword;  // 12bit;  // 0xfff 12bit

    // 12 + 4 = 16 bit
    uint16_t  id       : 1;
    uint16_t  layer    : 2;
    uint16_t  protection_absent : 1;

    // 16 + (2+4+1+3+1+1) = 16 + 12 = 28bit
    uint16_t  profile : 2;  // 0. main 1. lc 2. ssr
    // 0. 96khz 1. 88.2k 2. 64k 3. 48k 4. 44.1k 5. 32k 6. 24k 7. 22.05k 8. 16k 9. 12k 10. 11.025k 11. 8k
    uint16_t  sampling_frequency_index : 4;
    uint16_t  private_bit : 1;
    uint16_t  channel_configuration : 3;
    uint16_t  original : 1;
    uint16_t  home : 1;

    // Table 1. A.7 – Syntax of adts_variable_header()
    // adt varibable  header of adts
    // 2 + 13 + 11 + 2 = 28 bit
    uint16_t  copyright_identification_bit : 1;
    uint16_t  copyright_identification_start : 1;

    // aac_frame_length: Length of the frame including headers and error_check (Table A.8) in bytes.
    uint16_t  aac_frame_length : 13; // 16bit over contains header

    uint16_t  adts_buffer_fullness : 11;
    uint8_t   no_raw_data_blocks_in_frame : 2;

    // if protection_absent 16 bit 9bytes or 7bytes
    uint16_t crc16;
};
typedef std::shared_ptr<AdtsFixedHeader> PAdtsFixedHeader;

class AacAVCodecParser : public IAVCodecParser {
 public:
    AacAVCodecParser();

 public:
    error_t encode_avc(AVCodecContext* ctx, uint8_t* in_buf, int in_size,
                       std::list<PAVPacket>& pkts) override;

 public:
    PAdtsFixedHeader header;
};

}  // namespace sps

#endif  // SPS_AVCODEC_AAC_PARSER_HPP

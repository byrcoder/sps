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

#ifndef SPS_AVFORMAT_TS_HPP
#define SPS_AVFORMAT_TS_HPP

#include <list>
#include <map>
#include <memory>

#include <sps_cache_buffer.hpp>
#include <sps_io_bits.hpp>

#define SPS_TS_PACKET_SIZE 188

/* from ffmpeg pids */
#define PAT_PID         0x0000  /* Program Association Table */
#define CAT_PID         0x0001  /* Conditional Access Table */
#define TSDT_PID        0x0002  /* Transport Stream Description Table */
#define IPMP_PID        0x0003
/* PID from 0x0004 to 0x000F are reserved */
#define NIT_PID         0x0010  /* Network Information Table */
#define SDT_PID         0x0011  /* Service Description Table */
#define BAT_PID         0x0011  /* Bouquet Association Table */
#define EIT_PID         0x0012  /* Event Information Table */
#define RST_PID         0x0013  /* Running Status Table */
#define TDT_PID         0x0014  /* Time and Date Table */
#define TOT_PID         0x0014
#define NET_SYNC_PID    0x0015
#define RNT_PID         0x0016  /* RAR Notification Table */
/* PID from 0x0017 to 0x001B are reserved for future use
 * PID value 0x001C allocated to link-local inband signalling shall not be
 * used on any broadcast signals. It shall only be used between devices in a
 * controlled environment.
 **/
#define LINK_LOCAL_PID  0x001C
#define MEASUREMENT_PID 0x001D
#define DIT_PID         0x001E  /* Discontinuity Information Table */
#define SIT_PID         0x001F  /* Selection Information Table */
/* PID from 0x0020 to 0x1FFA may be assigned as needed to PMT, elementary
 * streams and other data tables
 */
#define FIRST_OTHER_PID 0x0020
#define  LAST_OTHER_PID 0x1FFA
/*
 * PID 0x1FFB is used by DigiCipher 2/ATSC MGT metadata
 * PID from 0x1FFC to 0x1FFE may be assigned as needed to PMT, elementary
 * streams and other data tables
 *
 */
/* Null packet (used for fixed bandwidth padding) */
#define NULL_PID        0x1FFF

#define TS_STREAM_TYPE_AUDIO_AAC        0x0F
#define TS_STREAM_TYPE_VIDEO_H264       0x1B
#define TS_STREAM_TYPE_VIDEO_H265       0x24


#define TS_DEFAULT_PES_PACKET_SIZE     1024 * 1024

namespace sps {

class TsPacket;
class TsProgram;
class TsContext;

typedef std::unique_ptr<TsPacket> PTsPacket;

// ts program has pes with data or psi
enum TsProgramType {
    TS_PROGRAM_UNKNOWN,
    TS_PROGRAM_PES,
    TS_PROGRAM_PAT,
    TS_PROGRAM_PMT,
    TS_PROGRAM_SDT,
};

// base ts program
class TsProgram : public std::enable_shared_from_this<TsProgram> {
 public:
    TsProgram(int pid, TsContext* ctx);

 public:
    virtual error_t decode(TsPacket* pkt, BitContext& rd) = 0;

    bool is_section();

 public:
    int pid;
    int es_id = 0;
    int last_cc = -1;
    int discard = -1;
    TsProgramType program_type = TS_PROGRAM_UNKNOWN;

 protected:
    TsContext* ctx;
};
typedef std::shared_ptr<TsProgram> PTsProgram;

// psi program
class TsPsiProgram : public TsProgram {
 public:
    TsPsiProgram(int pid, TsContext* ctx);

 public:
    error_t decode(TsPacket* pkt, BitContext& rd) override;

 protected:
    virtual error_t psi_decode(TsPacket* pkt, BitContext& rd) = 0;

 public:
    uint8_t pointer_field;
    /*
     * Table 2-26 – table_id assignment values
     * 0x00. program_association_section
     * 0x01. conditional_access_section
     * 0x02. TS_program_map_section
     * 0x03. TS_description_section
     * ..
     * 0x40-0xFE. User private
     * 0xFF. forbidden
     */
    uint8_t table_id;  // 8bit

    struct {
        uint32_t section_syntax_indicator : 1;  // 1bit
        uint32_t const_value0 : 1;  // 1bit
        uint32_t reserved : 2;  // 2bit
        uint32_t section_length : 12;  // 12bit
        // pat. self define
        // pmt. program_number
        // cat. reserved
        uint32_t transport_stream_id : 16;  // 16bit
    } flags1;

    struct {
        uint8_t reserved : 2;
        uint8_t version_number : 5;
        uint8_t current_next_indicator : 1;
    } flags2;

    uint8_t section_number;  // 8bit
    uint8_t last_section_number;  // 8bit

    /**
     * psi loop
     */

    uint8_t crc_32;  // 4 bit
};

class TsSdtProgram : public TsPsiProgram {
 public:
    TsSdtProgram(int pid, TsContext* ctx);

 public:
    error_t psi_decode(TsPacket* pkt, BitContext& rd) override;

 public:

    uint8_t reserved_future_use;
    uint16_t service_id;

    struct {
//        reserved_future_used : 6;
//        EIT_schedule_flag : 1;
//        EIT_present_following_flag : 1;
//        running_status : 3;
//        free_CA_mode : 1;
    };
};

struct PmtInfo {
    uint32_t program_number : 16;
    uint32_t reserved : 3;
    uint32_t program_map_pid : 13;
};

/**
 * The Program Association Table provides the correspondence
 * between a program_number and the PID value of the Transport Stream packets
 * which carry the program definition
 */
class TsPatProgram : public TsPsiProgram {
 public:
    TsPatProgram(int pid, TsContext* ctx);

 public:
    error_t psi_decode(TsPacket* pkt, BitContext& rd) override;

    std::list<PmtInfo> pmt_infos;
    /**
     * loop :
     *     program_number; // 16bit
     *     pmt_pid;  // 16 bit
     */
};

struct PesHeaderInfo {
    // Table 2-29 – Stream type assignments
    // https://en.wikipedia.org/wiki/Program-specific_information
    uint8_t  stream_type;  // 8bit
    uint16_t reserved1 : 3;
    uint16_t elementary_pid : 13;

    uint16_t reserved2 : 4;
    uint16_t es_info_length : 12;
};

struct PesInfo {
    PesHeaderInfo header;
    PCharBuffer pes_dec;
};
typedef std::shared_ptr<PesInfo> PPesInfo;

class TsPmtProgram : public TsPsiProgram {
 public:
    TsPmtProgram(int pid, TsContext* ctx);

 public:
    error_t psi_decode(TsPacket* pkt, BitContext& rd) override;

 public:
    struct {
        uint32_t reserved1 : 3;
        uint32_t pcr_pid : 13;
        uint32_t reserved2 : 4;

        uint32_t program_info_length_unused : 2;
        uint32_t program_info_length : 10;
    } pmt_flag;

    PCharBuffer program_info_desc;
    std::list<PPesInfo> pes_infos;
};

/**
 * ts context
 */
class TsPesContext {
 public:
    TsPesContext(TsContext* ctx, int stream_type);

 public:
    void init(int sz);
    error_t flush();

 private:
    void reset();

 public:
    error_t dump(BitContext& rd, int payload_unit_start_indicator);
    error_t on_payload_complete();

 public:
    int stream_type;
    int64_t pts = -1;
    int64_t dts = -1;
    uint16_t pes_packet_length = 0;
    uint32_t pes_packet_cap    = 0;
    PCharBuffer pes_packets;
    TsContext* ctx;
};
typedef std::unique_ptr<TsPesContext> PTsPesContext;

class TsPesProgram : public TsProgram {
 public:
    TsPesProgram(int pid, TsContext* ctx, int stream_type,
            int pcr_pid);

 public:
    error_t decode(TsPacket* pkt, BitContext& rd) override;

 private:
    struct {
        uint32_t packet_start_code_prefix : 24;  // 24bit
        uint32_t stream_id : 8;  // 8bit
        uint16_t pes_packet_length = 0;  // 16bit
    } pes_packet_header;

    struct {
        uint8_t const_value10 : 2;
        uint8_t pes_scrambling_control : 2;
        uint8_t pes_priority : 1;
        uint8_t data_alignment_indicator : 1;
        uint8_t copyright : 1;
        uint8_t original_or_copy : 1;

        uint8_t pts_dts_flags : 2;
        uint8_t escr_flag : 1;
        uint8_t es_rate_flag : 1;
        uint8_t dsm_trick_mode_flag : 1;
        uint8_t additional_copy_info_flag : 1;
        uint8_t pes_crc_flag : 1;
        uint8_t pes_extension_flag : 1;

        uint8_t pes_header_data_length;
    } pes_header;

    struct {
        uint8_t const_value_0010 : 4;
        uint8_t pts1 : 3;  // pts[32..30]
        uint8_t marker_bit1: 1;

        uint16_t pts2 : 15;  // PTS [29..15]
        uint16_t marker_bit2: 1;

        uint16_t pts3 : 15;  // PTS [14..0]
        uint16_t marker_bit3 : 1;
    } pts;

    struct {
        uint8_t const_value_0001 : 4;
        uint8_t dts1 : 3;  // DTS[32..30]
        uint8_t marker_bit1: 1;

        uint16_t dts2 : 15;  // DTS [29..15]
        uint16_t marker_bit2: 1;

        uint16_t dts3 : 15;  // DTS [14..0]
        uint16_t marker_bit3 : 1;
    } dts;

    // 48 bit 32bit + 16bit
    struct {
        uint8_t reserved;  // 2bit
        uint8_t escr_base1;  // 3bit  // ESCR_base[32..30]
        uint8_t marker_bit1;  // 1bit
        uint16_t escr_base2;   // 15bit   // ESCR_base[29..15]
        uint8_t marker_bit2;  // 1bit
        uint16_t escr_base3;   // 15bit  // ESCR_base[14..0]
        uint16_t marker_bit3;  // 1bit
        uint16_t escr_extension;  // 9bit
        uint8_t marker_bit4;     // 1bit
    } escr;

    struct {
        uint8_t marker_bit1;  // 1bit
        uint32_t es_rate;    // 22bit
        uint32_t mark_bit2;  // 1bit
    } es_rate;

    struct {
        uint8_t trick_mode_control : 3;

        union {
            struct {
                uint8_t field_id : 2;
                uint8_t intra_slice_refresh : 1;
                uint8_t frequency_truncation : 2;
            } fast_forward;

            struct {
                uint8_t rep_cntrl : 5;
            } slow_motion;

            struct {
                uint8_t field_id : 2;
                uint8_t reserved : 3;
            } freeze_frame;

            struct {
                uint8_t field_id : 2;
                uint8_t intra_slice_refresh : 1;
                uint8_t frequency_truncation : 2;
            } fast_reverse;

            struct {
                uint8_t rep_cntrl : 5;
            } slow_reverse;

            struct {
                uint8_t reserved : 5;
            } reserved;
        } flag;
    } dsm_trick_mode;

    struct {
        uint8_t marker_bit : 1;
        uint8_t additional_copy_info : 7;
    } additional_copy_info;

    struct {
        uint16_t previous_pes_packet_crc;
    } pes_crc;

    struct {
        struct {
            uint8_t pes_private_data_flag : 1;
            uint8_t pack_header_field_flag : 1;
            uint8_t program_packet_sequence_counter_flag : 1;
            uint8_t p_std_buffer_flag : 1;
            uint8_t reserved : 3;
            uint8_t pes_extension_flag_2 : 1;
        } flags;

        struct {
            PCharBuffer buffer;  // 128bit, 16bytes
        } pes_private_data;

        struct {
            uint8_t pack_field_length;  // 8bit
            PCharBuffer buffer;  //
        } pack_header_field;

        struct {
            uint8_t marker_bit1 : 1;
            uint8_t program_packet_sequence_counter : 7;

            uint8_t marker_bit2 : 1;
            uint8_t mpeg1_mpeg2_identifier : 1;
            uint8_t original_stuff_length : 6;
        } program_packet_sequence_counter;

        struct {
            uint16_t const_01 : 2; //'01'
            uint16_t p_std_buffer_scale : 1;
            uint16_t p_std_buffer_size : 13;
        } p_std_buffer;

        struct {
            struct {
                uint8_t marker_bit : 1;
                uint8_t pes_extension_field_length : 7;
            } header;

            PCharBuffer buffer;
        } pes_extension2;

    } pes_extension;

    PTsPesContext pes_ctx;
    int stream_type;
    int pcr_pid;
    uint8_t continuity_counter;
};

class IPesHandler {
 public:
    virtual error_t on_pes_complete(TsPesContext* pes) = 0;
};

class TsContext {
 public:
    explicit TsContext(IPesHandler *handler = nullptr);

 public:
    error_t decode(uint8_t* p, size_t len);

 public:
    PTsProgram get_program(int pid);
    error_t   add_program(int pid, PTsProgram program);

 public:
    error_t on_pes_complete(TsPesContext* pes);

 private:
    std::map<int, PTsProgram> filters;
    IPesHandler* handler;
};

/*
 * adaptation
 */
class TsAdaptationFiled {
 public:
    error_t decode(BitContext& rd);

 public:
    // The adaptation_field_length is an 8-bit field specifying the number
    // of bytes in the adaptation_field immediately
    // following the adaptation_field_length
    uint8_t adaption_field_length = 0;  // 8bit

    struct {
        // The discontinuity indicator is used to indicate two types
        // of discontinuities,
        // system time-base discontinuities and
        // continuity_counter discontinuities.
        uint8_t discontinuity_indicator : 1;  // 1bit
        uint8_t random_access_indicator : 1;  // 1bit
        uint8_t elementary_stream_priority_indicator : 1;  // 1bit
        uint8_t PCR_flag : 1;  // 1bit
        uint8_t OPCR_flag : 1;  // 1bit
        uint8_t splicing_point_flag : 1;  // 1bit
        uint8_t transport_private_data_flag : 1;  // 1bit
        uint8_t adaptation_field_extension_flag : 1;  // 1bit
    } flags;

    // PCR reference https://blog.csdn.net/zp704393004/article/details/82225892
    struct {
        uint64_t program_clock_reference_base;  // 33bit
        uint8_t  pcr_reserved;  // 6bit
        uint8_t program_clock_reference_extension;  // 9bit
    } pcr;
    uint64_t pcr_value = 0;

    // OPCR
    uint8_t opcr[6];
    //    uint64_t original_program_clock_reference_base;  // 33bit
    //    uint8_t  opcr_reserved;  // 6bit
    //    uint8_t  original_program_clock_reference_extension;  // 9bit

    // splicing
    uint8_t splice_countdown = 0;  // 8bit

    // transport_private_data
    uint8_t transport_private_data_length = 0;  // 8bit
    std::unique_ptr<uint8_t[]> transport_private_data;  // buffers

    // adaptation_field_extension
    uint8_t adaptation_field_extension_length;
    struct {
        uint8_t ltw_flag : 1;
        uint8_t piecewise_rate_flag : 1;
        uint8_t seamless_splice_flag : 1;
        uint8_t reserved : 5;
    } adaptation_field_extension_flags;
    struct {
        uint16_t ltw_valid_flag : 1;
        uint16_t ltw_offset : 15;
    } ltw;
    uint8_t piecewise_rate[3];
    //    uint8_t piecewise_rate_reserved;  // 2bit
    //    uint32_t piecewise_rate;  // 22bit
    uint8_t seamless_splice[5];
    //    uint8_t splice_type;  // 4bit
    //    uint8_t DTS_next_AU[32..30];  // 3bit
    //    uint8_t marker0_bit;  // 1bit
    //    uint16_t DTS_next_AU[29..15];  // 15bit
    //    uint8_t marker1_bit;  // 1bit
    //    uint16_t DTS_next_AU[14..0];  // 15bit
    //    uint8_t marker3_bit;  // 1bit
    uint16_t adaptation_field_extension_left_length = 0;  // skip the length

    uint16_t stuffing_length = 0;  // skip the length, all '11..'
};
typedef std::unique_ptr<TsAdaptationFiled> PAdaptationFiled;

class TsPacket {
 public:
    explicit TsPacket(TsContext* ctx, int packet_size = SPS_TS_PACKET_SIZE);

 public:
    error_t decode(BitContext& rd);

 private:
    int pkt_size;
    TsContext* ctx;

 public:
    uint8_t sync_byte = 0;  // 8bit 0x47
    uint8_t transport_error_indicator = 0;  // 1bit

    // 1. set 1 first byte of psi or pes, or set 0
    // 2. null packet set 0
    uint8_t payload_unit_start_indicator = 0;  // 1bit
    uint8_t transport_priority = 0;  // 1bit

    // 0x00. Program Association Table
    // 0x01. Conditional Access Table
    // 0x02. Transport Stream Description Table
    // 0x03~0x0F. reserved
    // others: pmt, elementary_PID..
    // 0x1FFF. null packet
    // The transport packets with PID values 0x0000, 0x0001,
    //      and 0x0010-0x1FFE are allowed to carry a PCR.
    uint16_t pid = 0;  // 13 bit

    // 00. no encrypt
    uint8_t transport_scrambling_control = 0;  // 2bit

    // 00. reserved for future ios/iec
    // 01. no adaption, payload only
    // 10. adaption only, no payload
    // 11. adaption followed by payload
    uint8_t adaptation_filed_control = 0;  // 2bit
    uint8_t continuity_counter = 0;  // 4bit

    PAdaptationFiled adaptation_field;
};

}  // namespace sps

#endif  // SPS_AVFORMAT_TS_HPP

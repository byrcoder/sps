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
// Created by byrcoder on 2021/7/25.
//

#include <sps_avformat_ts.hpp>

#include <utility>

#include <sps_log.hpp>

namespace sps {

const char* get_stream_name(int stream_type) {
    switch (stream_type) {
        case TS_STREAM_TYPE_AUDIO_AAC:
            return "aac";
        case TS_STREAM_TYPE_VIDEO_H264:
            return "h264";
        case TS_STREAM_TYPE_VIDEO_H265:
            return "h265";
        default:
            return "xx-stream-type";
    }
}

TsProgram::TsProgram(int pid, TsContext *ctx) {
    this->pid = pid;
    this->ctx = ctx;
}

bool TsProgram::is_section() {
    return program_type == TS_PROGRAM_PAT ||
        program_type == TS_PROGRAM_PMT  ||
        program_type == TS_PROGRAM_SDT;
}

TsPsiProgram::TsPsiProgram(int pid, TsContext *ctx) : TsProgram(pid, ctx) {
}

// work as ffmpeg parse_section_header
error_t TsPsiProgram::decode(TsPacket *pkt, BitContext& rd) {
    auto ret = rd.acquire(8);

    if (ret != SUCCESS) {
        sp_error("no enough size ret %d", ret);
        return ret;
    }

    if (pkt->payload_unit_start_indicator) {
        pointer_field = rd.read_int8();
    }

    // section header
    table_id = rd.read_int8();

    flags1.section_syntax_indicator = rd.read_bits(1);
    flags1.const_value0 = rd.read_bits(1);
    flags1.reserved = rd.read_bits(2);
    flags1.section_length = rd.read_bits(12);
    flags1.transport_stream_id = rd.read_bits(16);

    flags2.reserved = rd.read_bits(2);
    flags2.version_number = rd.read_bits(5);
    flags2.current_next_indicator = rd.read_bits(1);

    section_number = rd.read_int8();
    last_section_number = rd.read_int8();

    // 5 pre size, 4 crc32
    int left_length = flags1.section_length - 5 - 4;

    sp_debug("psi pid %d, section_syntax_indicator %x, const_value0 %x, reserved %x, "
            "section_length %x, transport_stream_id %x ",
            pid, flags1.section_syntax_indicator, flags1.const_value0, flags1.reserved,
            flags1.section_length, flags1.transport_stream_id);

    ret = psi_decode(pkt, rd);

    crc_32 = rd.read_int32();

    return ret;
}

TsSdtProgram::TsSdtProgram(int pid, TsContext *ctx) : TsPsiProgram(pid, ctx) {
}

error_t TsSdtProgram::psi_decode(TsPacket *pkt, BitContext &rd) {
    error_t ret = SUCCESS;

    reserved_future_use = rd.read_int8();

    return ret; // TODO FIXME
}

TsPatProgram::TsPatProgram(int pid, TsContext *ctx) : TsPsiProgram(pid, ctx) {
    program_type = TS_PROGRAM_PAT;
}

error_t TsPatProgram::psi_decode(TsPacket *pkt, BitContext &rd) {
    error_t ret = SUCCESS;
    // 5 pre size, 4 crc32
    int left_length = flags1.section_length - 5 - 4;

    pmt_infos.clear();
    if (left_length > 0) {
        ret = rd.acquire(left_length);

        if (ret != SUCCESS) {
            sp_error("no enough %d size ret %d", left_length, ret);
            return ret;
        }

        while (left_length > 0) {
            PmtInfo info;
            info.program_number  = rd.read_int16();
            info.reserved        = rd.read_bits(3);
            info.program_map_pid = rd.read_bits(13);

            pmt_infos.push_back(info);
            left_length -= 4;
        }
    }

    for (auto& pmt : pmt_infos) {
        sp_debug("pmt pid %d", pmt.program_map_pid);
        ctx->add_program(pmt.program_map_pid, std::make_shared<TsPmtProgram>(
                (int) pmt.program_map_pid, ctx
                ));
    }

    return ret;
}

TsPmtProgram::TsPmtProgram(int pid, TsContext *ctx) : TsPsiProgram(pid, ctx) {
    program_type = TS_PROGRAM_PMT;
}

error_t TsPmtProgram::psi_decode(TsPacket *pkt, BitContext &rd) {
    error_t ret = SUCCESS;

    if ((ret = rd.acquire(4)) != SUCCESS) {
        sp_error("not enough size ret %d", ret);
        return ret;
    }

    pmt_flag.reserved1 = rd.read_bits(3);
    pmt_flag.pcr_pid   = rd.read_bits(13);
    pmt_flag.reserved2 = rd.read_bits(4);
    pmt_flag.program_info_length_unused = rd.read_bits(2);
    pmt_flag.program_info_length = rd.read_bits(10);

    sp_debug("pmt_flag pid %d, reserved1 %x, pcr_pid %d, "
             "reserved2 %x,"
             "program_info_length_unused %d, program_info_length %d",
             pid, pmt_flag.reserved1, pmt_flag.pcr_pid,
             pmt_flag.reserved2,
             pmt_flag.program_info_length_unused,
             pmt_flag.program_info_length);


    if (pmt_flag.program_info_length > 0) {
        if ((ret = rd.acquire(pmt_flag.program_info_length)) != SUCCESS) {
            sp_error("not enough size %d ret %d", pmt_flag.program_info_length,
                      ret);
            return ret;
        }

        program_info_desc = CharBuffer::create_buf(
                pmt_flag.program_info_length);
        program_info_desc->append(rd.pos(), pmt_flag.program_info_length);
        rd.skip_read_bytes(pmt_flag.program_info_length);
    }
    // 5 pre size, 4 pmt_flag, program_info_length, 4 crc32
    int left_length = flags1.section_length - 5
            - 4 - pmt_flag.program_info_length - 4;

    pes_infos.clear();

    while (left_length > 0) {
        auto info = std::make_shared<PesInfo>();

        info->header.stream_type = rd.read_int8();
        info->header.reserved1   = rd.read_bits(3);
        info->header.elementary_pid = rd.read_bits(13);
        info->header.reserved2 = rd.read_bits(4);
        info->header.es_info_length = rd.read_bits(12);

        sp_debug("pmt got new stream_type %x, reserved1 %d, "
                "elementary_pid %d, reserved2 %x, es_info_length %d, "
                "left_size %d",
                info->header.stream_type, info->header.reserved1,
                info->header.elementary_pid, info->header.reserved2,
                info->header.es_info_length,
                left_length);

        if (info->header.es_info_length > 0) {
            if ((ret = rd.acquire(info->header.es_info_length)) != SUCCESS) {
                sp_error("not enough size %d ret %d",
                          info->header.es_info_length, ret);
                return ret;
            }

            info->pes_dec = CharBuffer::create_buf(
                    info->header.es_info_length);
            info->pes_dec->append(rd.pos(), info->header.es_info_length);
            rd.skip_read_bytes(info->header.es_info_length);
        }

        pes_infos.push_back(info);
        left_length -= (5 + info->header.es_info_length);
    }

    for (auto& pes_info : pes_infos) {
        switch (pes_info->header.stream_type) {
            case TS_STREAM_TYPE_AUDIO_AAC:
            case TS_STREAM_TYPE_VIDEO_H264:
            case TS_STREAM_TYPE_VIDEO_H265:
                // pat/pmt may send more than once
                if (ctx->get_program(pes_info->header.elementary_pid) == nullptr) {
                    ctx->add_program(pes_info->header.elementary_pid,
                                     std::make_shared<TsPesProgram>(
                                             (int) pes_info->header.elementary_pid,
                                             ctx, pes_info->header.stream_type,
                                             (int) pes_info->header.es_info_length));
                }
                break;
            default:
                sp_warn("not support stream_type %x, %d",
                        pes_info->header.stream_type,
                        pes_info->header.elementary_pid);
        }
    }

    return ret;
}

TsPesContext::TsPesContext(TsContext* ctx, int stream_type) {
    this->ctx = ctx;
    this->stream_type = stream_type;
}

void TsPesContext::reset() {
    pes_packets.reset();
    pts = -1;
    dts = -1;

    pes_packet_length = 0;
}

error_t TsPesContext::flush() {
    if (pes_packets) {
        return on_payload_complete();
    }

    return SUCCESS;
}

void TsPesContext::init(int sz) {
    pes_packet_length = sz;

    if (pes_packet_length > 0) {
        pes_packet_cap = pes_packet_length;
        pes_packets    = CharBuffer::create_buf(pes_packet_cap);
    } else {
        pes_packet_cap = TS_DEFAULT_PES_PACKET_SIZE;
        pes_packets    = CharBuffer::create_buf(pes_packet_cap);
    }

    pts = -1;
    dts = -1;
}

error_t TsPesContext::dump(BitContext &rd, int payload_unit_start_indicator) {
    error_t ret = SUCCESS;

    if (!pes_packets) {
        sp_warn("[%s] pes packet null pes_packet_length %d", get_stream_name(stream_type),
                pes_packet_length);
        rd.skip_read_bytes(rd.size());
        return SUCCESS;
    }

    size_t left_size = rd.size();
    if (pes_packet_length > 0) {
        left_size = std::min((size_t) pes_packet_length - pes_packets->size(), rd.size());
    }

    if (pes_packets->remain() < left_size) {
        sp_error("cap size %d remain %u < %lu", pes_packet_cap,
                 pes_packets->remain(), left_size);
        return ERROR_TS_PACKET_DECODE;
    }
    pes_packets->append(rd.pos(), left_size);
    rd.skip_read_bytes(left_size);
    auto p = pes_packets->buffer();
    sp_debug("[%s] start %d, left_size %ld (%ld), pes_length %d, pes_remain %d (%d),"
            "%x, %x, %x",
            get_stream_name(stream_type),
            payload_unit_start_indicator,
            left_size, rd.size(), pes_packet_length, pes_packets->remain(),
            pes_packet_length - pes_packets->size(),
            *p, *(p+1), *(p+2)
            );

    if (rd.size() > 0) {
        sp_warn("pes_packet_length %d has remain size %lu, %lu",
                pes_packet_length, rd.size(), left_size);
        rd.skip_read_bytes(rd.size());
    }
#if 0
    if (pes_packet_length > 0 && pes_packets->size() == pes_packet_length) {
        ret = flush();
    }
#endif

    return ret;
}

error_t TsPesContext::on_payload_complete() {
    error_t ret = SUCCESS;
    if (pes_packet_length == 0) {
        pes_packet_length = pes_packets->size();
    }

    sp_debug("[%s] pes recv complete stream_type 0x%4x, size %d, "
             "pes_packet_length %d, cap %d, "
             "pts %lld, dts %lld",
             get_stream_name(stream_type),
             stream_type,
             pes_packets->size(), pes_packet_length, pes_packets->cap(),
             pts/90, dts/90);

    ret = ctx->on_pes_complete(this);

    reset();
    return ret;
}

TsPesProgram::TsPesProgram(int pid, TsContext *ctx,
    int stream_type, int pcr_pid) : TsProgram(pid, ctx) {
    program_type       = TS_PROGRAM_PES;
    pes_ctx           = std::make_unique<TsPesContext>(ctx, stream_type);
    this->stream_type = stream_type;
    this->pcr_pid     = pcr_pid;
    this->continuity_counter = 0x0f;

    name = get_stream_name(stream_type);
}

error_t TsPesProgram::decode(TsPacket *pkt, BitContext &rd) {
    error_t ret = SUCCESS;

    if (pkt->payload_unit_start_indicator == 0 && !pes_ctx->pes_packets) {
        sp_error("[%s] pes packet is null", name);
        return ERROR_TS_PACKET_DECODE;
    }

    if (pkt->continuity_counter != (((int) continuity_counter + 1) & 0x0f)) {
        sp_error("[%s] ts pid %x, last cc %x, now %x",
                 name,
                 pid,
                 continuity_counter,
                 pkt->continuity_counter);
        return ERROR_TS_PACKET_CC_CONTINUE;
    }
    continuity_counter = pkt->continuity_counter;

    if (pkt->payload_unit_start_indicator == 0) {
        return pes_ctx->dump(rd, pkt->payload_unit_start_indicator);
    }

    if (pkt->payload_unit_start_indicator == 1) {
        if ((ret = rd.acquire(9)) != SUCCESS) {
            sp_error("not enough size %d, ret %d", 6, ret);
            return ret;
        }

        // pes pre tag flush
        if ((ret = pes_ctx->flush()) != SUCCESS) {
            sp_error("fail flush pre packet ret %d", ret);
            return ret;
        }

        pes_packet_header.packet_start_code_prefix = rd.read_int24();
        pes_packet_header.stream_id = rd.read_int8();
        pes_packet_header.pes_packet_length = rd.read_int16();

        // pes new packet
        pes_ctx->init(pes_packet_header.pes_packet_length);

        if (pes_packet_header.packet_start_code_prefix != 0x000001) {
            sp_error("packet_start_code_prefix %x not start with 0x01",
                     pes_packet_header.packet_start_code_prefix);
            return ERROR_TS_PACKET_DECODE;
        }
    }

    uint32_t code = (uint32_t) 0x0100 | pes_packet_header.stream_id;

    /*
     * program_stream_map, private_stream_2
     * ECM, EMM
     * program_stream_directory, DSMCC_stream
     * ITU-T Rec. H.222.1 type E stream, Padding stream
     */
    if (code != 0x1bc && code != 0x1bf &&
        code != 0x1f0 && code != 0x1f1 &&
        code != 0x1ff && code != 0x1f2 &&
        code != 0x1f8 && code != 0x1be) {

        pes_header.const_value10 = rd.read_bits(2);
        pes_header.pes_scrambling_control = rd.read_bits(2);
        pes_header.pes_priority = rd.read_bits(1);
        pes_header.data_alignment_indicator = rd.read_bits(1);
        pes_header.copyright = rd.read_bits(1);
        pes_header.original_or_copy = rd.read_bits(1);

        pes_header.pts_dts_flags = rd.read_bits(2);
        pes_header.escr_flag = rd.read_bits(1);
        pes_header.es_rate_flag = rd.read_bits(1);
        pes_header.dsm_trick_mode_flag = rd.read_bits(1);
        pes_header.additional_copy_info_flag = rd.read_bits(1);
        pes_header.pes_crc_flag = rd.read_bits(1);
        pes_header.pes_extension_flag = rd.read_bits(1);

        pes_header.pes_header_data_length = rd.read_int8();

        // sp_info("debug left %ld", rd.size());
        uint8_t* pos = rd.pos();

        pes_ctx->dts = pes_ctx->pts = -1;
        // pts flag
        if ((pes_header.pts_dts_flags & 0x02) > 0) {
            if ((ret = rd.acquire(5)) != SUCCESS) {
                sp_error("not enough size %d, ret %d", 6, ret);
                return ret;
            }

            pts.const_value_0010 = rd.read_bits(4);
            pts.pts1 = rd.read_bits(3);
            pts.marker_bit1 = rd.read_bits(1);

            pts.pts2 = rd.read_bits(15);
            pts.marker_bit2 = rd.read_bits(1);

            pts.pts3 = rd.read_bits(15);
            pts.marker_bit3 = rd.read_bits(1);

            int64_t tmp_pts = 0;
            tmp_pts |= (((uint64_t) pts.pts1) << 30) & 0x1c0000000LL;
            tmp_pts |= ((uint64_t) pts.pts2) << 15;
            tmp_pts |= ((uint64_t) pts.pts3);

            pes_ctx->dts = pes_ctx->pts = tmp_pts;
        }

        // dts flag
        if (pes_header.pts_dts_flags == 0x03) {
            if ((ret = rd.acquire(5)) != SUCCESS) {
                sp_error("not enough size %d, ret %d", 6, ret);
                return ret;
            }

            dts.const_value_0001 = rd.read_bits(4);
            dts.dts1 = rd.read_bits(3);
            dts.marker_bit1 = rd.read_bits(1);

            dts.dts2 = rd.read_bits(15);
            dts.marker_bit2 = rd.read_bits(1);

            dts.dts3 = rd.read_bits(15);
            dts.marker_bit3 = rd.read_bits(1);

            int64_t tmp_dts = 0;
            tmp_dts |= (((uint64_t) dts.dts1) << 30) & 0x1c0000000LL;
            tmp_dts |= ((uint64_t) dts.dts2) << 15;
            tmp_dts |= ((uint64_t) dts.dts3);

            pes_ctx->dts = tmp_dts;

            if (std::abs(pes_ctx->dts - pes_ctx->pts) > 90000) {
                sp_warn("[ts] sync dts=%lld, pts=%lld, diff > 1s", pes_ctx->dts, pes_ctx->pts);
            }
        }

        if (pes_header.escr_flag) {
            if ((ret = rd.acquire(6)) != SUCCESS) {
                sp_error("not enough size %d, ret %d", 6, ret);
                return ret;
            }
            uint8_t u1 = rd.read_int8();
            escr.reserved = (u1 & 0xc0) >> 6;
            escr.escr_base1 = (u1 & 0x38) >> 3;
            escr.marker_bit1 = (u1 & 0x04) >> 1;
            // remain 2bit

            uint16_t u2 = rd.read_int16();
            escr.escr_base2 = (((uint32_t) (u1 & 0x03)) << 13) | ((u2 & 0xFFF8) >> 3);
            escr.marker_bit2 = (u2 & 0x040) >> 1;
            // remain 2 bit

            uint16_t u3 = rd.read_int16();
            escr.escr_base3 = (((uint32_t) (u2 & 0x03)) << 13) | ((u3 & 0xFFF8) >> 3);
            escr.marker_bit3 = (u3 & 0x040) >> 1;
            // remain 2bit

            uint8_t u4 = rd.read_int8();
            escr.escr_extension = (((uint16_t) (u3 & 0x03)) << 7) | ((u4 & 0xFE) >> 1);
            escr.marker_bit4 = u4 & 0x01;
        }

        if (pes_header.es_rate_flag) {
            if ((ret = rd.acquire(3)) != SUCCESS) {
                sp_error("not enough size %d, ret %d", 3, ret);
                return ret;
            }

            uint32_t u1 = rd.read_int24();
            es_rate.marker_bit1 = u1 & 0x800000;
            es_rate.es_rate = u1 & 0x7ffffe;
            es_rate.mark_bit2 = u1 & 0x01;
        }

        if (pes_header.dsm_trick_mode_flag) {
            if ((ret = rd.acquire(1)) != SUCCESS) {
                sp_error("not enough size %d, ret %d", 1, ret);
                return ret;
            }

            // TODO: FIXME
            rd.skip_read_bytes(1);
            // rd.read_bytes((uint8_t*) &dsm_trick_mode, 1);
        }

        if (pes_header.additional_copy_info_flag) {
            if ((ret = rd.acquire(1)) != SUCCESS) {
                sp_error("not enough size %d, ret %d", 1, ret);
                return ret;
            }

            // TODO: FIXME
            rd.skip_read_bytes(1);
            // rd.read_bytes((uint8_t*) &additional_copy_info, 1);
        }

        if (pes_header.pes_crc_flag) {
            if ((ret = rd.acquire(2)) != SUCCESS) {
                sp_error("not enough size %d, ret %d", 2, ret);
                return ret;
            }

            pes_crc.previous_pes_packet_crc = rd.read_int16();
        }

        if (pes_header.pes_extension_flag) {
            if ((ret = rd.acquire(1)) != SUCCESS) {
                sp_error("not enough size %d, ret %d", 1, ret);
                return ret;
            }

            pes_extension.flags.pes_private_data_flag = rd.read_bits(1);
            pes_extension.flags.pack_header_field_flag = rd.read_bits(1);
            pes_extension.flags.program_packet_sequence_counter_flag = rd.read_bits(1);
            pes_extension.flags.p_std_buffer_flag = rd.read_bits(1);
            pes_extension.flags.reserved = rd.read_bits(3);
            pes_extension.flags.pes_extension_flag_2 = rd.read_bits(1);

            if (pes_extension.flags.pes_private_data_flag) {
                if ((ret = rd.acquire(16)) != SUCCESS) {
                    sp_error("not enough size %d, ret %d", 16, ret);
                    return ret;
                }

                pes_extension.pes_private_data.buffer = CharBuffer::create_buf(16);
                pes_extension.pes_private_data.buffer->append(rd.pos(), 16);
                rd.skip_read_bytes(16);
            }

            if (pes_extension.flags.pack_header_field_flag) {
                if ((ret = rd.acquire(1)) != SUCCESS) {
                    sp_error("not enough size %d, ret %d", 1, ret);
                    return ret;
                }

                pes_extension.pack_header_field.pack_field_length = rd.read_int8();
                pes_extension.pack_header_field.buffer = CharBuffer::create_buf(
                        pes_extension.pack_header_field.pack_field_length);

                if ((ret = rd.acquire(pes_extension.pack_header_field.pack_field_length)) != SUCCESS) {
                    sp_error("not enough size %d, ret %d", 1, ret);
                    return ret;
                }

                pes_extension.pack_header_field.buffer->append(rd.pos(),
                        pes_extension.pack_header_field.pack_field_length);
                rd.skip_read_bytes(pes_extension.pack_header_field.pack_field_length);
            }

            if (pes_extension.flags.program_packet_sequence_counter_flag) {
                if ((ret = rd.acquire(2)) != SUCCESS) {
                    sp_error("not enough size %d, ret %d", 2, ret);
                    return ret;
                }

                // TODO: FIMXE
                rd.skip_read_bytes(2);
                // rd.read_bytes((uint8_t*) &pes_extension.program_packet_sequence_counter, 2);
            }

            if (pes_extension.flags.p_std_buffer_flag) {
                if ((ret = rd.acquire(2)) != SUCCESS) {
                    sp_error("not enough size %d, ret %d", 2, ret);
                    return ret;
                }

                // TODO: FIMXE
                rd.skip_read_bytes(2);
                // rd.read_bytes((uint8_t*) &pes_extension.p_std_buffer, 2);
            }

            if (pes_extension.flags.pes_extension_flag_2) {
                if ((ret = rd.acquire(1)) != SUCCESS) {
                    sp_error("not enough size %d, ret %d", 1, ret);
                    return ret;
                }

                pes_extension.pes_extension2.header.marker_bit = rd.read_bits(1);
                pes_extension.pes_extension2.header.pes_extension_field_length = rd.read_bits(7);

                if ((ret = rd.acquire(pes_extension.pes_extension2.header.pes_extension_field_length)) != SUCCESS) {
                    sp_error("not enough size %d, ret %d",
                            pes_extension.pes_extension2.header.pes_extension_field_length,
                            ret);
                    return ret;
                }

                pes_extension.pes_extension2.buffer = CharBuffer::create_buf(
                        pes_extension.pes_extension2.header.pes_extension_field_length);

                pes_extension.pes_extension2.buffer->append(rd.pos(),
                        pes_extension.pes_extension2.header.pes_extension_field_length);
                rd.skip_read_bytes(pes_extension.pes_extension2.header.pes_extension_field_length);
            }
        }

        uint8_t* end_pos = rd.pos();
        int stuff_head_size = pes_header.pes_header_data_length - (end_pos - pos);

        // sp_info("[%s] debug left %ld", name, rd.size());
        if ((ret = rd.acquire(stuff_head_size)) != SUCCESS) {
            sp_error("not enough size %d, ret %d", stuff_head_size, ret);
            return ret;
        }

        rd.skip_read_bytes(stuff_head_size);
        // sp_info("[%s] debug left %ld", name, rd.size());

        if ((ret = pes_ctx->dump(rd, pkt->payload_unit_start_indicator)) != SUCCESS) {
            sp_error("fail dump pes ret %d", ret);
            return ret;
        }

        sp_debug("[%s] got %s pes packet_start_code_prefix %x, "
                "stream_id %x, pes_packet_length %d, \r\n"
                ""
                "pes_scrambling_control %x, "
                "pes_priority %x, "
                "data_alignment_indicator %x, "
                "copyright %x,"
                "original_or_copy %x, "
                "pts_dts_flags %x, "
                "escr_flag %x, "
                "es_rate_flag %x, "
                "dsm_trick_mode_flag %x, "
                "additional_copy_info_flag %x, "
                "pes_crc_flag %x, "
                "pes_extension_flag %x, "
                "pes_header_data_length %x, "
                "pts %lld, dts %lld, "
                "stuff_head_size %d, ",
                name,
                pkt->payload_unit_start_indicator ? "new" : "old",
                pes_packet_header.packet_start_code_prefix,
                pes_packet_header.stream_id,
                pes_packet_header.pes_packet_length,
                pes_header.pes_scrambling_control,
                pes_header.pes_priority,
                pes_header.data_alignment_indicator,
                pes_header.copyright,
                pes_header.original_or_copy,

                pes_header.pts_dts_flags,
                pes_header.escr_flag,
                pes_header.es_rate_flag,
                pes_header.dsm_trick_mode_flag,
                pes_header.additional_copy_info_flag,
                pes_header.pes_crc_flag,
                pes_header.pes_extension_flag,
                pes_header.pes_header_data_length,
                pes_ctx->pts, pes_ctx->dts,
                stuff_head_size
        );
    } else if (code == 0x1be) {  // padding stream
        rd.skip_read_bytes(rd.size());
    } else if (code == 0x1bc || code == 0x1bf ||
               code == 0x1f0 || code == 0x1f1 ||
               code == 0x1ff || code == 0x1f2 ||
               code == 0x1f8) {
        if ((ret = pes_ctx->dump(rd, pkt->payload_unit_start_indicator)) != SUCCESS) {
            sp_error("fail dump pes ret %d", ret);
            return ret;
        }
    }

    return ret;
}

TsContext::TsContext(IPesHandler* handler,
                     PITsPacketHandler packet_handler) {
    this->handler = handler;
    this->pkt_handler = std::move(packet_handler);
    add_program(PAT_PID, std::make_shared<TsPatProgram>(PAT_PID, this));
    add_program(SDT_PID, std::make_shared<TsSdtProgram>(SDT_PID, this));
    // add_filter(EIT_PID, nullptr);
}

error_t TsContext::decode(uint8_t* p, size_t len) {
    BitContext rd(p, len);
    TsPacket pkt(this);
    error_t  ret = pkt.decode(rd);

    if (ret != SUCCESS) {
        sp_error("fail decode pkt ret %d", ret);
        return ret;
    }

    auto program = get_program(pkt.pid);
    if (!program) {
        sp_warn("ignore pid %d unknown type", pkt.pid);
        return ret;
    }

    // has no payload
    if ((pkt.adaptation_filed_control & 0x01) == 0 && !program->is_section()) {
        return ret;
    }

    if ((ret = program->decode(&pkt, rd)) != SUCCESS) {
        sp_error("fail decode payload ret %d", ret);
        return ret;
    }

    if (pkt_handler &&
        (ret = pkt_handler->on_packet(p, len, program->program_type,
                pkt.payload_unit_start_indicator)) != SUCCESS) {
        sp_error("fail handler ts packet ret %d", ret);
        return ret;
    }

    return ret;
}

PTsProgram TsContext::get_program(int pid) {
    auto it = filters.find(pid);
    return it == filters.end() ? nullptr : it->second;
}

error_t TsContext::add_program(int pid, PTsProgram program) {
    auto old_filter = get_program(pid);

    if (old_filter) {
        sp_debug("replace old_filter pid %d", pid);
    }
    filters[pid] = std::move(program);

    return SUCCESS;
}

error_t TsContext::on_pes_complete(TsPesContext *pes) {
    if (handler) {
        return handler->on_pes_complete(pes);
    }
    return SUCCESS;
}

error_t TsAdaptationFiled::decode(BitContext& rd) {
    auto ret = rd.acquire(1);

    if (ret != SUCCESS) {
        return ret;
    }
    adaption_field_length = rd.read_int8();

    uint8_t *pos = rd.pos();

    if (adaption_field_length <= 0) {
        return ret;
    }

    ret = rd.acquire(adaption_field_length);

    if (ret != SUCCESS) {
        return ret;
    }

    flags.discontinuity_indicator = rd.read_bits(1);
    flags.random_access_indicator = rd.read_bits(1);
    flags.elementary_stream_priority_indicator = rd.read_bits(1);
    flags.PCR_flag = rd.read_bits(1);
    flags.OPCR_flag = rd.read_bits(1);
    flags.splicing_point_flag = rd.read_bits(1);
    flags.transport_private_data_flag = rd.read_bits(1);
    flags.adaptation_field_extension_flag = rd.read_bits(1);

    if (flags.PCR_flag) {
        pcr.program_clock_reference_base = rd.read_bits(33);
        pcr.pcr_reserved = rd.read_bits(6);
        pcr.program_clock_reference_extension = rd.read_bits(9);
        pcr_value = pcr.program_clock_reference_base * 300 + pcr.program_clock_reference_extension;
    }

    if (flags.OPCR_flag) {
        rd.read_bytes(opcr, 6);
    }

    if (flags.splicing_point_flag) {
        splice_countdown = rd.read_int8();
    }

    if (flags.adaptation_field_extension_flag) {
        transport_private_data_length = rd.read_int8();

        if ((ret = rd.acquire(transport_private_data_length)) != SUCCESS) {
            return ret;
        }

        transport_private_data = std::make_unique<uint8_t[]>(
                transport_private_data_length);
        rd.read_bytes(transport_private_data.get(),
                       transport_private_data_length);
    }

    // stuffing_byte
    stuffing_length = adaption_field_length - (rd.pos() - pos);
    rd.skip_read_bytes(stuffing_length);

    return ret;
}

TsPacket::TsPacket(TsContext* ctx, int packet_size) {
    this->ctx      = ctx;
    this->pkt_size = packet_size;
}

error_t TsPacket::decode(BitContext& rd) {
    error_t ret = SUCCESS;

    if ((ret = rd.acquire(pkt_size)) != SUCCESS) {
        sp_error("not match packet size %d, actual %lu",
                  pkt_size, rd.size());
        return ERROR_TS_PACKET_SIZE;
    }

    // 4byte for header
    sync_byte = rd.read_int8();
    if (sync_byte != 0x47) {
        sp_error("sync_byte %x not 0x47", sync_byte);
        return ERROR_TS_PACKET_DECODE;
    }

    transport_error_indicator = rd.read_bits(1);
    payload_unit_start_indicator = rd.read_bits(1);
    transport_priority = rd.read_bits(1);
    pid = rd.read_bits(13);

    transport_scrambling_control = rd.read_bits(2);
    adaptation_filed_control = rd.read_bits(2);
    continuity_counter = rd.read_bits(4);

    if (adaptation_filed_control & 0x02) {
        adaptation_field = std::make_unique<TsAdaptationFiled>();
        ret = adaptation_field->decode(rd);
        if (ret != SUCCESS) {
            return ret;
        }
#if 0
        sp_info("pid %d, adaptation discontinuity_indicator %x, "
                "random_access_indicator %x, "
                "elementary_stream_priority_indicator %x, "
                "PCR_flag %x, OPCR_flag %x, "
                "splicing_point_flag %x, transport_private_data_flag: %x, "
                "adaptation_field_extension_flag %x, "
                "pcr_value %llu, ",
                pid, adaptation_field->flags.discontinuity_indicator,
                adaptation_field->flags.random_access_indicator,
                adaptation_field->flags.elementary_stream_priority_indicator,
                adaptation_field->flags.PCR_flag,
                adaptation_field->flags.OPCR_flag,
                adaptation_field->flags.splicing_point_flag,
                adaptation_field->flags.transport_private_data_flag,
                adaptation_field->flags.adaptation_field_extension_flag,
                adaptation_field->pcr_value);
#endif
    }

#if 0
    if (payload_unit_start_indicator && !adaptation_field) {
        sp_info("ts pkt sync_byte %x,  transport_error_indicator %d, "
                "payload_unit_start_indicator %d, transport_priority %d, "
                "pid %d, transport_scrambling_control %d, "
                "continuity_counter: %2d, adaptation_filed_control: %x",
                sync_byte, transport_error_indicator,
                payload_unit_start_indicator, transport_priority,
                pid, transport_scrambling_control,
                continuity_counter, adaptation_filed_control);
    } else if (payload_unit_start_indicator) {
        sp_info("ts pkt sync_byte %x,  transport_error_indicator %d, "
                "payload_unit_start_indicator %d, transport_priority %d, "
                "pid %d, transport_scrambling_control %d, "
                "continuity_counter: %2d, adaptation_filed_control: %x "
                "ts adaption adaption_field_length %d, "
                "PCR_flag %d, OPCR_flag %d, splicing_point_flag %d, "
                "adaptation_field_extension_flag %d",
                sync_byte, transport_error_indicator,
                payload_unit_start_indicator, transport_priority,
                pid, transport_scrambling_control,
                continuity_counter, adaptation_filed_control,
                adaptation_field->adaption_field_length,
                adaptation_field->flags.PCR_flag,
                adaptation_field->flags.OPCR_flag,
                adaptation_field->flags.splicing_point_flag,
                adaptation_field->flags.adaptation_field_extension_flag);
    }
#endif
    return ret;
}

}  // namespace sps

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

TsProgram::TsProgram(int pid, TsContext *ctx) {
    this->pid = pid;
    this->ctx = ctx;
}

TsPsiProgram::TsPsiProgram(int pid, TsContext *ctx) : TsProgram(pid, ctx) {
}

error_t TsPsiProgram::decode(TsPacket *pkt, PSpsBytesReader &rd) {
    auto ret = rd->acquire(8);

    if (ret != SUCCESS) {
        sp_error("no enough size ret %d", ret);
        return ret;
    }

    table_id = rd->read_int8();
    rd->read_bytes((uint8_t*) &flags1, 4);
    rd->read_bytes((uint8_t*) &flags2, 1);
    section_number = rd->read_int8();
    last_section_number = rd->read_int8();

    // 5 pre size, 4 crc32
    int left_length = flags1.section_length - 5 - 4;

    ret = psi_decode(pkt, rd);

    crc_32 = rd->read_int32();

    return ret;
}

TsPatProgram::TsPatProgram(int pid, TsContext *ctx) : TsPsiProgram(pid, ctx) {
    filter_type = TS_PROGRAM_PAT;
}

error_t TsPatProgram::psi_decode(TsPacket *pkt, PSpsBytesReader &rd) {
    error_t ret = SUCCESS;
    // 5 pre size, 4 crc32
    int left_length = flags1.section_length - 5 - 4;

    pmt_infos.clear();
    if (left_length > 0) {
        ret = rd->acquire(left_length);

        if (ret != SUCCESS) {
            sp_error("no enough %d size ret %d", left_length, ret);
            return ret;
        }

        while (left_length > 0) {
            PmtInfo info;
            rd->read_bytes((uint8_t*) &info, 4);
            pmt_infos.push_back(info);
            left_length -= 4;
        }
    }
    return ret;
}

TsPmtProgram::TsPmtProgram(int pid, TsContext *ctx) : TsPsiProgram(pid, ctx) {
    filter_type = TS_PROGRAM_PMT;
}

error_t TsPmtProgram::psi_decode(TsPacket *pkt, PSpsBytesReader &rd) {
    error_t ret = SUCCESS;

    if ((ret = rd->acquire(4)) != SUCCESS) {
        sp_error("not enough size ret %d", ret);
        return ret;
    }

    rd->read_bytes((uint8_t*) &pmt_flag, 4);

    if (pmt_flag.program_info_length > 0) {
        if ((ret = rd->acquire(pmt_flag.program_info_length)) != SUCCESS) {
            sp_error("not enough size %d ret %d", pmt_flag.program_info_length,
                      ret);
            return ret;
        }

        program_info_desc = CharBuffer::create_buf(
                pmt_flag.program_info_length);
        program_info_desc->append(rd->pos(), pmt_flag.program_info_length);
        rd->skip_bytes(pmt_flag.program_info_length);
    }
    // 5 pre size, 4 pmt_flag, program_info_length, 4 crc32
    int left_length = flags1.section_length - 5
            - 4 - pmt_flag.program_info_length - 4;

    pes_infos.clear();

    while (left_length > 0) {
        auto info = std::make_shared<PesInfo>();

        rd->read_bytes((uint8_t*) &info->header, sizeof(info->header));
        if (info->header.es_info_length > 0) {
            if ((ret = rd->acquire(info->header.es_info_length)) != SUCCESS) {
                sp_error("not enough size %d ret %d",
                          info->header.es_info_length, ret);
                return ret;
            }

            info->pes_dec = CharBuffer::create_buf(
                    info->header.es_info_length);
            info->pes_dec->append(rd->pos(), info->header.es_info_length);
            rd->skip_bytes(info->header.es_info_length);
        }

        pes_infos.push_back(info);
        left_length -= (4 + info->header.es_info_length);
    }

    for (auto& pes_info : pes_infos) {
        switch (pes_info->header.stream_type) {
            case TS_STREAM_TYPE_AUDIO_AAC:
            case TS_STREAM_TYPE_VIDEO_H264:
            case TS_STREAM_TYPE_VIDEO_H265:
                ctx->add_program(pes_info->header.elementary_pid,
                     std::make_shared<TsPesProgram>(
                                 (int) pes_info->header.elementary_pid,
                                 ctx, pes_info->header.stream_type,
                                 (int) pes_info->header.es_info_length));
                break;
            default:
                sp_warn("not support stream_type %x, %d",
                        pes_info->header.stream_type,
                        pes_info->header.elementary_pid);
        }
    }

    return ret;
}

void TsPesContext::reset() {
    pts = -1;
    dts = -1;
    pes_packet_length = 0;
    pes_packets.reset();
}

void TsPesContext::init() {
    pes_packets = CharBuffer::create_buf(pes_packet_length);
}

error_t TsPesContext::dump(PSpsBytesReader &rd) {
    if (!pes_packets && pes_packet_length > 0) {
        pes_packets = CharBuffer::create_buf(pes_packet_length);
    }

    if (!pes_packets) {
        sp_warn("pes packet null pes_packet_length %d", pes_packet_length);
        rd->skip_bytes(rd->size());
        return SUCCESS;
    }

    size_t left_size = std::min((size_t) pes_packet_length, rd->size());
    pes_packets->append(rd->pos(), left_size);
    rd->skip_bytes(left_size);

    if (rd->size() > 0) {
        sp_warn("has remain size %lu", rd->size());
        rd->skip_bytes(rd->size());
    }

    return SUCCESS;
}

TsPesProgram::TsPesProgram(int pid, TsContext *ctx,
    int stream_type, int pcr_pid) : TsProgram(pid, ctx) {
    filter_type       = TS_PROGRAM_PES;
    pes_ctx           = std::make_unique<TsPesContext>();
    this->stream_type = stream_type;
    this->pcr_pid     = pcr_pid;
}

error_t TsPesProgram::decode(TsPacket *pkt, PSpsBytesReader &rd) {
    error_t ret = SUCCESS;

    if (pkt->payload_unit_start_indicator == 0 && !pes_ctx->pes_packets) {
        sp_error("pes packet is null");
        return ERROR_TS_PACKET_DECODE;
    }

    if (pkt->payload_unit_start_indicator == 0) {
        return pes_ctx->dump(rd);
    }

    if (pkt->payload_unit_start_indicator == 1) {
        if ((ret = rd->acquire(9)) != SUCCESS) {
            sp_error("not enough size %d, ret %d", 6, ret);
            return ret;
        }

        pes_ctx->reset();

        rd->read_bytes((uint8_t*) &pes_packet_header, 6);
        pes_ctx->pes_packet_length = pes_packet_header.pes_packet_length;

        pes_ctx->init();

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

        rd->read_bytes((uint8_t*) &pes_header, 3);

        uint8_t* pos = rd->pos();

        if ((pes_header.pts_dts_flags & 0x02) > 0) {
            if ((ret = rd->acquire(5)) != SUCCESS) {
                sp_error("not enough size %d, ret %d", 6, ret);
                return ret;
            }

            rd->read_bytes((uint8_t*) &pts, 5);

            int64_t tmp_pts = 0;
            tmp_pts |= (((uint64_t) pts.pts1) << 30) & 0x1c0000000LL;
            tmp_pts |= ((uint64_t) pts.pts2) << 15;
            tmp_pts |= ((uint64_t) pts.pts3);

            pes_ctx->dts = pes_ctx->pts = tmp_pts;
        }

        if (pes_header.pts_dts_flags == 0x03) {
            if ((ret = rd->acquire(5)) != SUCCESS) {
                sp_error("not enough size %d, ret %d", 6, ret);
                return ret;
            }

            rd->read_bytes((uint8_t*) &dts, 5);

            int64_t tmp_dts = 0;
            tmp_dts |= (((uint64_t) dts.dts1) << 30) & 0x1c0000000LL;
            tmp_dts |= ((uint64_t) dts.dts2) << 15;
            tmp_dts |= ((uint64_t) dts.dts3);

            pes_ctx->dts = tmp_dts;

            if (std::abs(pes_ctx->dts - pes_ctx->pts) > 90000) {
                sp_warn("ts: sync dts=%lld pts=%lld", pes_ctx->dts, pes_ctx->pts);
            }
        }

        if (pes_header.escr_flag) {
            if ((ret = rd->acquire(6)) != SUCCESS) {
                sp_error("not enough size %d, ret %d", 6, ret);
                return ret;
            }
            uint8_t u1 = rd->read_int8();
            escr.reserved = (u1 & 0xc0) >> 6;
            escr.escr_base1 = (u1 & 0x38) >> 3;
            escr.marker_bit1 = (u1 & 0x04) >> 1;
            // remain 2bit

            uint16_t u2 = rd->read_int16();
            escr.escr_base2 = (((uint32_t) (u1 & 0x03)) << 13) | ((u2 & 0xFFF8) >> 3);
            escr.marker_bit2 = (u2 & 0x040) >> 1;
            // remain 2 bit

            uint16_t u3 = rd->read_int16();
            escr.escr_base3 = (((uint32_t) (u2 & 0x03)) << 13) | ((u3 & 0xFFF8) >> 3);
            escr.marker_bit3 = (u3 & 0x040) >> 1;
            // remain 2bit

            uint8_t u4 = rd->read_int8();
            escr.escr_extension = (((uint16_t) (u3 & 0x03)) << 7) | ((u4 & 0xFE) >> 1);
            escr.marker_bit4 = u4 & 0x01;
        }

        if (pes_header.es_rate_flag) {
            if ((ret = rd->acquire(3)) != SUCCESS) {
                sp_error("not enough size %d, ret %d", 3, ret);
                return ret;
            }

            uint32_t u1 = rd->read_int24();
            es_rate.marker_bit1 = u1 & 0x800000;
            es_rate.es_rate = u1 & 0x7ffffe;
            es_rate.mark_bit2 = u1 & 0x01;
        }

        if (pes_header.dsm_trick_mode_flag) {
            if ((ret = rd->acquire(1)) != SUCCESS) {
                sp_error("not enough size %d, ret %d", 1, ret);
                return ret;
            }

            rd->read_bytes((uint8_t*) &dsm_trick_mode, 1);
        }

        if (pes_header.additional_copy_info_flag) {
            if ((ret = rd->acquire(1)) != SUCCESS) {
                sp_error("not enough size %d, ret %d", 1, ret);
                return ret;
            }

            rd->read_bytes((uint8_t*) &additional_copy_info, 1);
        }

        if (pes_header.pes_crc_flag) {
            if ((ret = rd->acquire(2)) != SUCCESS) {
                sp_error("not enough size %d, ret %d", 2, ret);
                return ret;
            }

            pes_crc.previous_pes_packet_crc = rd->read_int16();
        }

        if (pes_header.pes_extension_flag) {
            if ((ret = rd->acquire(1)) != SUCCESS) {
                sp_error("not enough size %d, ret %d", 1, ret);
                return ret;
            }

            rd->read_bytes((uint8_t*) &pes_extension.flags, 1);

            if (pes_extension.flags.pes_private_data_flag) {
                if ((ret = rd->acquire(16)) != SUCCESS) {
                    sp_error("not enough size %d, ret %d", 16, ret);
                    return ret;
                }

                pes_extension.pes_private_data.buffer = CharBuffer::create_buf(16);
                pes_extension.pes_private_data.buffer->append(rd->pos(), 16);
                rd->skip_bytes(16);
            }

            if (pes_extension.flags.pack_header_field_flag) {
                if ((ret = rd->acquire(1)) != SUCCESS) {
                    sp_error("not enough size %d, ret %d", 1, ret);
                    return ret;
                }

                pes_extension.pack_header_field.pack_field_length = rd->read_int8();
                pes_extension.pack_header_field.buffer = CharBuffer::create_buf(
                        pes_extension.pack_header_field.pack_field_length);

                if ((ret = rd->acquire(pes_extension.pack_header_field.pack_field_length)) != SUCCESS) {
                    sp_error("not enough size %d, ret %d", 1, ret);
                    return ret;
                }

                pes_extension.pack_header_field.buffer->append(rd->pos(),
                        pes_extension.pack_header_field.pack_field_length);
                rd->skip_bytes(pes_extension.pack_header_field.pack_field_length);
            }

            if (pes_extension.flags.program_packet_sequence_counter_flag) {
                if ((ret = rd->acquire(2)) != SUCCESS) {
                    sp_error("not enough size %d, ret %d", 2, ret);
                    return ret;
                }

                rd->read_bytes((uint8_t*) &pes_extension.program_packet_sequence_counter, 2);
            }

            if (pes_extension.flags.p_std_buffer_flag) {
                if ((ret = rd->acquire(2)) != SUCCESS) {
                    sp_error("not enough size %d, ret %d", 2, ret);
                    return ret;
                }

                rd->read_bytes((uint8_t*) &pes_extension.p_std_buffer, 2);
            }

            if (pes_extension.flags.pes_extension_flag_2) {
                if ((ret = rd->acquire(1)) != SUCCESS) {
                    sp_error("not enough size %d, ret %d", 1, ret);
                    return ret;
                }

                rd->read_bytes((uint8_t*) &pes_extension.pes_extension2.header, 1);

                if ((ret = rd->acquire(pes_extension.pes_extension2.header.pes_extension_field_length)) != SUCCESS) {
                    sp_error("not enough size %d, ret %d",
                            pes_extension.pes_extension2.header.pes_extension_field_length,
                            ret);
                    return ret;
                }

                pes_extension.pes_extension2.buffer = CharBuffer::create_buf(
                        pes_extension.pes_extension2.header.pes_extension_field_length);

                pes_extension.pes_extension2.buffer->append(rd->pos(),
                        pes_extension.pes_extension2.header.pes_extension_field_length);
                rd->skip_bytes(pes_extension.pes_extension2.header.pes_extension_field_length);
            }
        }

        uint8_t* end_pos = rd->pos();
        int stuff_head_size = pes_header.pes_header_data_length-(end_pos-pos);

        if ((ret = rd->acquire(stuff_head_size)) != SUCCESS) {
            sp_error("not enough size %d, ret %d", stuff_head_size, ret);
            return ret;
        }

        rd->skip_bytes(stuff_head_size);

        if ((ret = pes_ctx->dump(rd)) != SUCCESS) {
            sp_error("fail dump pes ret %d", ret);
            return ret;
        }
    } else if (code == 0x1be) {  // padding stream
        rd->skip_bytes(rd->size());
    } else if (code == 0x1bc || code == 0x1bf ||
               code == 0x1f0 || code == 0x1f1 ||
               code == 0x1ff || code == 0x1f2 ||
               code == 0x1f8) {
        if ((ret = pes_ctx->dump(rd)) != SUCCESS) {
            sp_error("fail dump pes ret %d", ret);
            return ret;
        }
    }

    return ret;
}

TsContext::TsContext() {
    add_program(PAT_PID, std::make_shared<TsPatProgram>(PAT_PID, this));
    // add_filter(SDT_PID, nullptr);
    // add_filter(EIT_PID, nullptr);
}

error_t TsContext::decode(uint8_t* p, size_t len) {
    auto rd = SpsBytesReader::create_reader(p, len);
    TsPacket pkt(this);
    error_t  ret = pkt.decode(rd);

    if (ret != SUCCESS) {
        sp_error("fail decode pkt ret %d", ret);
        return ret;
    }

    // has no payload
    if ((pkt.adaptation_filed_control & 0x01) != 0) {
        return ret;
    }

    auto filter = get_program(pkt.pid);
    if (!filter) {
        sp_warn("ignore pid %d unknown type", pkt.pid);
        return ret;
    }

    if ((ret = filter->decode(&pkt, rd)) != SUCCESS) {
        sp_error("fail decode payload ret %d", ret);
        return ret;
    }

    return ret;
}

PTsProgram TsContext::get_program(int pid) {
    auto it = filters.find(pid);
    return it == filters.end() ? nullptr : it->second;
}

error_t TsContext::add_program(int pid, PTsProgram filter) {
    auto old_filter = get_program(pid);

    if (old_filter) {
        sp_warn("replace old_filter pid %d", pid);
    }
    filters[pid] = std::move(filter);

    return SUCCESS;
}

error_t TsAdaptationFiled::decode(PSpsBytesReader& rd) {
    auto ret = rd->acquire(1);

    if (ret != SUCCESS) {
        return ret;
    }
    adaption_field_length = rd->read_int8();

    uint8_t *pos = rd->pos();

    if (adaption_field_length <= 0) {
        return ret;
    }

    ret = rd->acquire(adaption_field_length);

    if (ret != SUCCESS) {
        return ret;
    }

    rd->read_bytes((uint8_t*) &flags, 1);

    if (flags.PCR_flag) {
        rd->read_bytes(pcr, 6);
    }

    if (flags.OPCR_flag) {
        rd->read_bytes(opcr, 6);
    }

    if (flags.splicing_point_flag) {
        splice_countdown = rd->read_int8();
    }

    if (flags.adaptation_field_extension_flag) {
        transport_private_data_length = rd->read_int8();

        if ((ret = rd->acquire(transport_private_data_length)) != SUCCESS) {
            return ret;
        }

        transport_private_data = std::make_unique<uint8_t[]>(
                transport_private_data_length);
        rd->read_bytes(transport_private_data.get(),
                       transport_private_data_length);
    }

    // stuffing_byte
    stuffing_length = adaption_field_length - (rd->pos() - pos);
    rd->skip_bytes(stuffing_length);

    return ret;
}

TsPacket::TsPacket(TsContext* ctx, int packet_size) {
    this->ctx      = ctx;
    this->pkt_size = packet_size;
}

error_t TsPacket::decode(PSpsBytesReader& rd) {
    error_t ret = SUCCESS;

    if ((ret = rd->acquire(pkt_size)) != SUCCESS) {
        sp_error("not match packet size %d, actual %lu",
                  pkt_size, rd->size());
        return ERROR_TS_PACKET_SIZE;
    }

    sync_byte = rd->read_int8();
    if (sync_byte != 0x47) {
        sp_error("sync_byte %x not 0x47", sync_byte);
        return ERROR_TS_PACKET_DECODE;
    }

    uint16_t tmp = rd->read_int16();
    transport_error_indicator = (tmp & 0x80) >> 15;
    payload_unit_start_indicator = (tmp & 0x40) >> 14;
    transport_priority = (tmp & 0x20) >> 13;
    pid = tmp & 0x1F;

    tmp = rd->read_int8();
    transport_scrambling_control = (tmp & 0xC0) >> 6;
    adaptation_filed_control = (tmp & 0x30) >> 4;
    continuity_counter = tmp & 0x0F;

    if (adaptation_filed_control & 0x02) {
        adaptation_field = std::make_unique<TsAdaptationFiled>();
        ret = adaptation_field->decode(rd);
        if (ret != SUCCESS) {
            return ret;
        }
    }

    return ret;
}

}  // namespace sps

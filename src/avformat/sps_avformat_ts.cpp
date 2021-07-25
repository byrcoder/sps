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

        program_info_desc = std::make_unique<uint8_t[]>(
                pmt_flag.program_info_length);
        rd->read_bytes(program_info_desc.get(), pmt_flag.program_info_length);
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

            info->pes_desc = std::make_unique<uint8_t[]>(
                    info->header.es_info_length);
            rd->read_bytes(info->pes_desc.get(), info->header.es_info_length);
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

TsPesProgram::TsPesProgram(int pid, TsContext *ctx,
    int stream_type, int pcr_pid) : TsProgram(pid, ctx) {
    filter_type = TS_PROGRAM_PES;
    pes_ctx     = std::make_unique<TsPesContext>();
    this->stream_type = stream_type;
    this->pcr_pid     = pcr_pid;
}

error_t TsPesProgram::decode(TsPacket *pkt, PSpsBytesReader &rd) {
    error_t ret = SUCCESS;
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

    uint8_t *pos = rd->buf->pos();

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
    stuffing_length = adaption_field_length - (rd->buf->pos() - pos);
    rd->skip_bytes(stuffing_length);

    return ret;
}

TsPacket::TsPacket(TsContext* ctx, int packet_size) {
    this->ctx      = ctx;
    this->pkt_size = packet_size;
}

error_t TsPacket::decode(PSpsBytesReader& rd) {
    error_t ret = SUCCESS;

    if (rd->acquire(pkt_size)) {
        sp_error("not match packet size %d, actual %lu",
                  pkt_size, rd->buf->size());
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

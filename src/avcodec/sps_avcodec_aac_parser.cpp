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
// Created by byrcoder on 2021/8/4.
//

#include <sps_avcodec_aac_parser.hpp>

#include <sps_io_bits.hpp>
#include <sps_log.hpp>
#include <sps_error.hpp>

namespace sps {

AdtsFixedHeader::AdtsFixedHeader() {
    syncword = 0;
    id = 0;
    layer = 0;
    protection_absent = 0;
    profile = 0;
    sampling_frequency_index = 0;
    private_bit = 0;
    channel_configuration = 0;
    original = 0;
    home = 0;

    copyright_identification_bit = 0;
    copyright_identification_start = 0;

    aac_frame_length = 0;
    adts_buffer_fullness = 0;
    no_raw_data_blocks_in_frame = 0;
    crc16 = 0;
}

bool AdtsFixedHeader::operator==(const AdtsFixedHeader &header) {
    return protection_absent = header.protection_absent &&
            profile == header.profile &&
            sampling_frequency_index == header.sampling_frequency_index &&
                    channel_configuration == header.channel_configuration &&
                    original == header.original &&
                    home == header.home;
}

error_t AdtsFixedHeader::decode(BitContext *bc) {
    if (bc->acquire(7) != SUCCESS) {
        sp_error("aac size < 7");
        return ERROR_AAC_DECODER;
    }

    syncword = bc->read_bits(12);
    if (syncword != 0xfff) {
        sp_error("aac sync header not 0xfff %x", syncword);
        return ERROR_AAC_DECODER;
    }

    id = bc->read_bits(1);
    layer = bc->read_bits(2);
    protection_absent = bc->read_bits(1);

    profile = bc->read_bits(2);
    sampling_frequency_index = bc->read_bits(4);
    private_bit = bc->read_bits(1);
    channel_configuration = bc->read_bits(3);
    original = bc->read_bits(1);
    home = bc->read_bits(1);

    copyright_identification_bit = bc->read_bits(1);
    copyright_identification_start = bc->read_bits(1);
    aac_frame_length = bc->read_bits(13);

    adts_buffer_fullness = bc->read_bits(11);
    no_raw_data_blocks_in_frame = bc->read_bits(2);

    if (!protection_absent) {
        crc16 = bc->read_bits(16);
    }

    return SUCCESS;
}

AacAVCodecParser::AacAVCodecParser() {
    header = nullptr;
}

error_t AacAVCodecParser::encode_avc(AVCodecContext *ctx, uint8_t *in_buf,
                                     int in_size, std::list<PAVPacket> &pkts) {
    error_t ret = SUCCESS;
    BitContext bc(in_buf, in_size);

    AdtsFixedHeader tmp_header;
    if ((ret = tmp_header.decode(&bc)) != SUCCESS) {
        return ret;
    }

    if (!header || !(*header == tmp_header)) {
        if (!header) {
            header = std::make_unique<AdtsFixedHeader>();
        }

        *header = tmp_header;

        uint8_t aac_header[4];
        uint8_t& flags = aac_header[0];

        uint8_t sound_rate = 0;
        // For FLV, only support 5, 11, 22, 44KHz sampling rate.
        if (header->sampling_frequency_index <= 0x0c && header->sampling_frequency_index > 0x0a) {
            sound_rate = 0;
        } else if (header->sampling_frequency_index <= 0x0a && header->sampling_frequency_index > 0x07) {
            sound_rate = 1;
        } else if (header->sampling_frequency_index <= 0x07 && header->sampling_frequency_index > 0x04) {
            sound_rate = 2;
        } else if (header->sampling_frequency_index <= 0x04) {
            sound_rate = 3;
        } else {
            sound_rate = 4;
        }

        uint8_t sound_type = std::max(0, (std::min(header->channel_configuration - 1, 1))); // sound_type
        uint8_t sound_size = 0x01; // sound size

        flags = AAC << 4;
        flags |= (sound_rate << 2);
        flags |= (sound_size << 1);
        flags |= (sound_type << 1);

        // aac to flv
        aac_header[1] = ((header->profile + 1) << 3) & 0xf8;  // aac header 5 bit
        aac_header[1] = (header->sampling_frequency_index >> 1) >> 0x07;
        aac_header[2] = (header->sampling_frequency_index << 7) & 0x80;
        aac_header[2] = (header->channel_configuration << 7) & 0x78;

        // channels    = flags & 0x01    // (0. SndMomo 1. SndStero)
        // sample_size = flags & 0x02    // (0. 8       1. 16)
        // sample_rate = flags & 0x0c    // (0. 5.5k  1. 11k   2. 22k   3. 44k)

        auto pkt = AVPacket::create(
                AV_MESSAGE_DATA,
                AV_STREAM_TYPE_AUDIO,
                AVPacketType {AV_AUDIO_TYPE_SEQUENCE_HEADER},
                aac_header,
                sizeof(aac_header),
                0,
                0,
                flags,
                AAC
                );
        pkts.push_back(pkt);
    }

    return ret;
}

}
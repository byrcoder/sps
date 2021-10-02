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
// Created by byrcoder on 2021/10/1.
//

#include <sps_avformat_rtpdec_aac.hpp>
#include <sps_log.hpp>

namespace sps {

// RFC 3640
// aac_parse_packet->ff_mpeg4_generic_dynamic_handler
/**
  *
  *    +---------+-----------+-----------+---------------+
  *    | RTP     | AU Header | Auxiliary | Access Unit   |
  *    | Header  | Section   | Section   | Data Section  |
  *    +---------+-----------+-----------+---------------+
  *              <----------RTP Packet Payload----------->
  *
  *     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- .. -+-+-+-+-+-+-+-+-+-+
  *     |AU-headers-length|AU-header|AU-header|      |AU-header|padding|
  *     |        2-byte   |   (1)   |   (2)   |      |   (n)   | bits  |
  *     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- .. -+-+-+-+-+-+-+-+-+-+
  *              Figure 2: The AU Header Section
  *
  *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- .. -+-+-+-+-+-+-+-+-+
  *    | auxiliary-data-size   | auxiliary-data       |padding bits |
  *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- .. -+-+-+-+-+-+-+-+-+
  *              Figure 4: The fields in the Auxiliary Section
  */
error_t RtpAacAuHeader::decode(BitContext &bc) {
    const int au_header_size = 2;

    error_t ret = SUCCESS;

    if ((ret = bc.acquire(2)) != SUCCESS) {
        sp_error("[rtp@aac] fail acquire ret %d", ret);
        return ret;
    }

    au_headers_length = bc.read_int16();  // bits
    int au_headers_byte   = (au_headers_length + 7) / 8;
    nb_au_headers     = au_headers_byte / au_header_size;

    if ((ret = bc.acquire(au_header_size * nb_au_headers)) != SUCCESS) {
        sp_error("[rtp@aac] fail acquire ret %d", ret);
        return ret;
    }

    for (int i = 0; i < nb_au_headers; ++i) {
        RtpAacAu au;
        au.au_size  = bc.read_bits(13);
        au.au_index = bc.read_bits(3);

        au_lists.push_back(au);
    }

    return ret;
}

AacRtpDecoder::AacRtpDecoder() {
}

error_t AacRtpDecoder::decode(RtpPayloadHeader &header, BitContext &bc,
                              std::list<PAVPacket> &pkts) {
    AVCodecContext ctx(header.fix_header.timestamp, header.fix_header.timestamp, 90);
    RtpAacAuHeader au_header;
    error_t ret = SUCCESS;

    if ((ret = au_header.decode(bc)) != SUCCESS) {
        return ret;
    }

    for (auto& au : au_header.au_lists) {
        if ((ret = bc.acquire(au.au_size)) != SUCCESS) {
            sp_error("[rtp@aac] fail acquire %u, ret %d", au.au_size, ret);
            return ret;
        }

        if ((ret = aac_parser.encode_raw_avc(&ctx, nullptr, bc.pos(), au.au_size, pkts)) != SUCCESS) {
            sp_error("[rtp@aac] decode ret %d", ret);
            return ret;
        }
        bc.skip_read_bytes(au.au_size);
    }

    return ret;
}

}
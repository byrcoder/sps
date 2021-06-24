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

#ifndef SPS_RTMP_HANDSHAKE_HANDLER_HPP
#define SPS_RTMP_HANDSHAKE_HANDLER_HPP

#include <sps_host_phase_handler.hpp>

namespace sps {

/*
 * c0: rtmp version default 0x03, 0x06 encrypt(not used)
 * c1: 1536-bytes consists:
 *     4-bytes timestamp,
 *     4-bytes version, version zero simple handshake or complex handshake.
 *     simple-hk: 1528-bytes random data
 *     complex-hk: key and digest maybe swap
 *              key: 764-bytes
 *                   764-bytes-key:
 *                           random-data:  offset-bytes
 *                           key-data:     128-bytes （public key for dh)
 *                           random-data:  (764-offset-128-4)-bytes
 *                           offset:       4-bytes
 *              digest: 764-bytes
 *                      764-bytes-digest:
 *                           offset:        4-bytes
 *                           random-data:   offset-bytes
 *                           digest-data:   32-bytes
 *                           random-data:   (764-4-offset-32) bytes
 *              c1-complex-hk:
 *              -----------------------------------------------------------
 *              | timestamp |  version |    digest         |   key        |
 *              -----------------------------------------------------------
 *
 *              digest:
 *              -------------------------------------------------------------
 *              | offset    |   random-data0  |  digest-data | random-data1 |
 *              -------------------------------------------------------------
 *              c1-s1-join-data = join-bytes(offset, rand-data0, rand-data1)
 *              digest-data (32-bytes) = HMACsha256(c1-s1-join, FPKey, 30)
 *
 *              key:
 *              ------------------------------------------------------------
 *              | random-data |   key-data     |   random-data |   offset  |
 *              ------------------------------------------------------------
 *
 *  s1: c1 is simple-handshake, echo c1
 *      c1 is complex-handshake:
 *            c1-key-data:   c1 key-data
 *
 *            -----------------------------------------------------------
 *            | timestamp | version |  digest  |        key             |
 *            -----------------------------------------------------------
 *
 *            digest:
 *           ----------------------------------------------------------------
 *           | offset    |   random-data0  |  s1-digest-data | random-data1 |
 *           ----------------------------------------------------------------
 *
 *            key:
 *            ---------------------------------------------------------------
 *            | random-data |   s1-key-data     |   random-data |   offset  |
 *            ---------------------------------------------------------------
 *
 *            s1-key-data    = shared_key = DH_compute_key(c1-key-data)
 *            s1-digest-data = HMACsha256(c1-s1-join-data, FMSKey, 36)
 *
 *
 *  c2: simple-hk: random 1536
 *      complex-hk:
 *            random-data: 1504-bytes
 *            c2-digest-data: 32-bytes
 *            ----------------------------------------------------------------
 *            | random-data                            | c2-digest-data      |
 *            ----------------------------------------------------------------
 *                random fill c2s2 1536 bytes
 *                tmp_Key           = HMACsha256(s1-digest-data, FPKey, 62)
 *                c2-digest-data = HMACsha256(c2-random-data, tmp_key, 32)
 *
 * s2: simple-hk: echo c2
 *     complex-hk:
 *            random-data: 1504-bytes
 *            s2-digest-data: 32-bytes
 *            ----------------------------------------------------------------
 *            | random-data                            | c2-digest-data      |
 *            ----------------------------------------------------------------
 *             random fill c2s2 1536 bytes
 *             tmp_Key           = HMACsha256(c1-digest-data, FMSKey, 68)
 *             c2-digest-data = HMACsha256(c2-random-data, tmp_key, 32)
 */

// rtmp handshake and connect
class RtmpHandshakeHandler : public IPhaseHandler {
 public:
    RtmpHandshakeHandler();

 public:
    error_t handler(HostPhaseCtx& ctx) override;
};

}

#endif  // SPS_RTMP_HANDSHAKE_HANDLER_HPP
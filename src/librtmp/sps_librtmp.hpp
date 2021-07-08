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

#ifndef SPS_LIBRTMP_HPP
#define SPS_LIBRTMP_HPP

#include <librtmp/rtmp.h>

#include <cstring>
#include <string>
#include <memory>

#include <sps_io.hpp>
#include <librtmp/sps_librtmp_packet.hpp>

/**
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

/**
 * 1. rtmp chunk format
 * +--------------+----------------+--------------------+--------------+
 * | Basic Header | Message Header | Extended Timestamp | Chunk Data   |
 * +--------------+----------------+--------------------+--------------+
 * |<------------------- Chunk Header ----------------->|
 *
 * Basic Header:
 * +-----------------------------------+
 * | fmt 2-bit| chunk-id               |
 * +-----------------------------------+
 *
 * chunk-id = [2, 63],  then 6 bit chunk-id, ranges [2, 63]
 *  ----------------------------------------------
 * | fmt 2-bit| chunk-id (6-bit)                |
 * ----------------------------------------------
 *
 * chunk-id = 0, then chunk-id = 64 + the second byte, ranges  [64, 319]
 * --------------------------------------------------------
 * | fmt 2-bit| 0 (6-bit)        |  the second byte       |
 * --------------------------------------------------------
 *
 * chunk-id = 1,       then chunk-id = 64 + the second byte + (the third byte)*256,  ranges [64, 65599]
 * --------------------------------------------------------------------------
 * | fmt 2-bit| 1 (6-bit)        |  the second byte    |  the third byte    |
 * --------------------------------------------------------------------------
 *
 * Message Header:
 *  fmt = 0, 11-bytes, timestamp is absolute time
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  | timestamp (3-byte)                    | message length 3-bytes|
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-++-+-+-+-+
 *  | message length (as upon)      |message type id (1-byte) | msg stream id|
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-++-+-+-+-+
 *  | message stream id (3-bytes)   |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-++-+-+-+-+
 *  | extend_timestamp (4-bytes)   (optional  timestamp=0XFFFFF )            |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-++-+-+-+-+
 *
 * fmt = 1, no message stream id, same as before, timestamp is delta
 * fmt = 2, no message stream id, message-type-id, length, timestamp is delta
 * fmt = 3, no message stream id, length, timestamp
 * extend_time only happend in fmt = [0,1,2], never fmt=3
 *
 * 2. message type
 * message-type = [1,2,3,5,6] is ctrl msg, and stream_id must be 2
 *      [set-chunked, abort, ack, set-win-size-ack, set-peer-band)
 * message-type = 4, is user ctrl msg
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-++-+-+-+-+
 *  | event_type (2-bytes) |  event_data                                     |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-++-+-+-+-+
 *        [StreamBegin,  StreamEOF..., PING, PONG, SetBuffer]
 *
 * message-type = 8, audio
 * message-type = 9, video
 * message-type = [15, 17] amf0 data and cmd, [18, 20] amf3 data and cmd
 *
 */

/**
 * 1. RTMP Chunk Stream uses message type IDs 1, 2, 3, 5, and 6 for
 * protocol control messages.
 * These protocol control messages MUST have message stream ID 0 (known
 * as the control stream) and be sent in chunk stream ID 2
 *
 * 2. User Control messages SHOULD use message stream ID 0 (known as the
 * control stream) and, when sent over RTMP Chunk Stream, be sent on
 * chunk stream ID 2
 **/
namespace sps {

void librtmp_init_once();

class RtmpHook {
 public:
    explicit RtmpHook(PIReaderWriter io);

    ~RtmpHook();

 public:
    void set_recv_timeout(utime_t tm);

    void set_send_timeout(utime_t tm);

    RTMP *get_rtmp();

    error_t get_error() const;

 private:
    /**
     * set rtmp number value when send
     */
    error_t on_send_packet(RTMPPacket &packet);

    /**
     * process rtmp invoke when recv packet
     * 1. set chunked size
     * 2. ping/pong
     * 3. ack win
     * 4. client/server bandwidth
     */
    error_t on_recv_packet(RTMPPacket &packet);

 public:
    error_t recv_packet(WrapRtmpPacket &packet);

    error_t send_packet(WrapRtmpPacket &packet, bool queue);

    error_t send_packet(RTMPPacket &packet, bool queue);

 public:
    error_t send_result(double txn, double id);

    // chunked id = 2
    error_t send_ack_window_size();

    // set chunked size for output
    error_t send_set_chunked_size();

    error_t send_ping(uint32_t /** stream_id **/);

    error_t send_pong(uint32_t /** stream_id **/);

    error_t ack();

 public:
    // rtmp server->client
    error_t server_handshake();

    error_t send_peer_bandwidth();

    error_t send_play_start(int /** transaction_id **/);

    error_t send_publish_start(int transaction_id);

    // flash
    error_t send_sample_access();

    error_t send_stream_begin();

    error_t send_connect_result(double txn);

 public:
    // rtmp client->server
    error_t client_connect(const std::string &url, const std::string &params,
                           bool publish);

    error_t send_server_bandwidth();

    error_t send_connect(const std::string &app, const std::string &tc_url);

    error_t create_stream();

    error_t send_buffer_length();

 private:
    PIReaderWriter skt;
    RTMP *rtmp;
    RTMP_HOOK hook{0};
    error_t error;
    int transaction_id = 0;
    int next_acked;
    int buffer_ms      = 2000;

 public:
    // connect, RTMP_CONNECT, FALSE=0 TRUE=1
    static int SPS_RTMP_Connect(RTMP *r, RTMPPacket *cp);

    static int SPS_RTMP_TLS_Accept(RTMP *r, void *ctx);

    // send ret < 0 fail, n byte return ok
    static int SPS_RTMPSockBuf_Send(RTMPSockBuf *sb, const char *buf, int len);

    // return -1 error, n byte return success
    static int SPS_RTMPSockBuf_Fill(RTMPSockBuf *sb, char *buf, int nb_bytes);

    // close
    static int SPS_RTMPSockBuf_Close(RTMPSockBuf *sb);

    // search
    static int SPS_RTMP_IsConnected(RTMP *r);

    static int SPS_RTMP_Socket(RTMP *r);
};

typedef std::shared_ptr<RtmpHook> PRtmpHook;

}  // namespace sps

#endif  // SPS_LIBRTMP_HPP

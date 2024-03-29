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

#ifndef SPS_SPS_ERROR_HPP
#define SPS_SPS_ERROR_HPP

#include <sps_auto_header.hpp>

#define SUCCESS 0L

#define ERROR_CO_CREATE                     50

#define ERROR_CONFIG_ITEM                 91
#define ERROR_CONFIG_SUBMODULE            92
#define ERROR_CONFIG_PARSE_INVALID        93

#define ERROR_CONFIG_OPT_SET              100
#define ERROR_CONFIG_OPT_TYPE             101
#define ERROR_CONFIG_FACTORY_DUP          102

#define ERROR_CONFIG_INSTALL              103

#define ERROR_MODULE_TYPE_NOT_MATCH       200

// tcp socket 操作错误吗 1000～1200 tcp错误
#define ERROR_SOCKET_CREATE                 1000
#define ERROR_SOCKET_SETREUSE               1001
#define ERROR_SOCKET_BIND                   1002
#define ERROR_SOCKET_LISTEN                 1003
#define ERROR_SOCKET_CLOSED                 1004
#define ERROR_SOCKET_READ                   1007
#define ERROR_SOCKET_READ_FULLY             1008
#define ERROR_SOCKET_WRITE                  1009
#define ERROR_SOCKET_TIMEOUT                1011
#define ERROR_SOCKET_EINTR                  1065
#define ERROR_SOCKET_BUFFER_SMALL           1066

#define ERROR_ST_OPEN_SOCKET                1200
#define ERROR_ST_CONNECT                    1201

// srt 错误
#define ERROR_SRT_SOCKET_CREATE             1300
#define ERROR_SRT_SOCKET_CONNECT            1301
#define ERROR_SRT_SOCKET_TIMEOUT            1304
#define ERROR_SRT_SOCKET_CLOSED             1305
#define ERROR_SRT_SOCKET_BROKEN             1306
#define ERROR_SRT_SOCKET_CLOSING            1307
#define ERROR_SRT_SOCKET_NOTEXITS           1308
#define ERROR_SRT_SOCKET_STATUS_OK          1309
#define ERROR_SRT_SOCKET_BIND               1310
#define ERROR_SRT_SOCKET_LISTEN             1311
#define ERROR_SRT_SOCKET_UNKNOWN            1320
#define ERROR_SRT_NOT_SUPPORT_FAST_OPEN     1321
#define ERROR_SRT_STREAM_ID                 1322

// ssl error
#define ERROR_SSL_READ                      1330
#define ERROR_SSL_WRITE                     1331
#define ERROR_SSL_HANDSHAKE                 1332
#define ERROR_SSL_HANDSHAKE_CONTINUE        1333
#define ERROR_SSL_CERT_NOT_FOUND            1334

#define ERROR_URL_PROTOCOL_NOT_EXISTS       1340

#define ERROR_FILE_OPEN                     1350
#define ERROR_FILE_READ                     1351
#define ERROR_FILE_WRITE                    1352
#define ERROR_IO_EOF                        1353
#define ERROR_IO_BUFFER_FULL                1354
#define ERROR_IO_NOT_ENOUGH                 1355

// DNS 等解析错误
#define ERROR_SYSTEM_IP_INVALID             1500

// SYSTEM 内存
#define ERROR_MEM_OVERFLOW                  1600
#define ERROR_MEM_SOCKET_READ               1601
#define ERROR_MEM_SOCKET_WRITE              1602

// HTTP 错误
#define ERROR_HTTP_SOCKET_CREATE            1700
#define ERROR_HTTP_SOCKET_CONNECT           1701
#define ERROR_HTTP_HEADER_PARSE             1703
#define ERROR_HTTP_RSP_NOT_OK               1704
#define ERROR_HTTP_RES_EOF                  1705
#define ERROR_HTTP_UPSTREAM_NOT_FOUND       1706
#define ERROR_HTTP_CHUNKED_LENGTH_LARGE     1707
#define ERROR_HTTP_CHUNKED_INVALID          1708
#define ERROR_HTTP_HAS_SOURCE               1709

// HOST 错误
#define ERROR_HOST_NOT_EXISTS               1800

#define ERROR_UPSTREAM_NOT_FOUND            1900
#define ERROR_RTMP_NOT_IMPL                 1901
#define ERROR_RTMP_NOT_CONNECT              1902
#define ERROR_RTMP_HANDSHAKE                1903
#define ERROR_RTMP_AMF_DECODE               1904
#define ERROR_RTMP_AMF_CMD_CONVERT          1905
#define ERROR_RTMP_AMF_PROP_NOT_FOUND       1906
#define ERROR_RTMP_AMF_PROP_TYPE            1907
#define ERROR_RTMP_CONNECT                  1908
#define ERROR_RTMP_ROLE                     1909
#define ERROR_RTMP_NO_SOURCE                1910
#define ERROR_RTMP_HAS_SOURCE               1911
#define ERROR_RTMP_HEAD_TOO_SHORT           1912;


// stream 协议
#define ERROR_STREAM_EOF                    3000
#define ERROR_STREAM_NOT_IMPL               3001
#define ERROR_STREAM_NOT_CONF               3002
#define ERROR_STREAM_IO_FAIL                3003
#define ERROR_STREAM_PUBLISH_NOT_SUPPORT    3004
#define ERROR_STREAM_SOURCE_NOT_EXITS       3005

#define ERROR_STREAM_EDGE_NOT_START         3010

#define ERROR_AVFORMAT_ENCODER_NOT_EXISTS   3100
#define ERROR_AVFORMAT_DEMUX_NOT_EXISTS     3101
#define ERROR_AVFORMAT_RTMP_IO_NULL         3104

// flv 协议
#define ERROR_FLV_PROBE                     3201
#define ERROR_FLV_BUFFER_OVERFLOW           3202
#define ERROR_FLV_TAG                       3203
#define ERROR_FLV_AUDIO_CODECID             3204
#define ERROR_FLV_VIDEO_CODECID             3205
#define ERROR_FLV_UNKNOWN_TAG_TYPE          3206

#define ERROR_TS_PACKET_SIZE                3300
#define ERROR_TS_PACKET_DECODE              3301
#define ERROR_TS_PACKET_CC_CONTINUE         3302

#define ERROR_RTP_PROBE                     3350
#define ERROR_RTP_NOT_SUPPORT               3351
#define ERROR_RTP_PAYLOAD_TYPE              3352
#define ERROR_RTP_DECODER_H264              3353

#define ERROR_H264_DECODER                  3500
#define ERROR_H264_ENCODER                  3501
#define ERROR_H264_NOT_IMPL                 3502

#define ERROR_AAC_DECODER                   3600

#ifdef FFMPEG_ENABLED

#define ERROR_FFMPEG_OPEN                   5000
#define ERROR_FFMPEG_READ                   5001
#define ERROR_FFMPEG_WRITE                  5002

#endif

#endif  // SPS_SPS_ERROR_HPP

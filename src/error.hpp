#ifndef SPS_ERROR_HPP
#define SPS_ERROR_HPP

#define ERROR_CONFIG_EOF                  90
#define ERROR_CONFIG_ITEM                 91
#define ERROR_CONFIG_SUBMODULE            92
#define ERROR_CONFIG_PARSE_INVALID        93

#define ERROR_CONFIG_OPT_SET              100
#define ERROR_CONFIG_OPT_TYPE             101
#define ERROR_CONFIG_FACTORY_DUP          102

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

#define ERROR_FILE_OPEN                     1350
#define ERROR_IO_EOF                        1351
#define ERROR_FILE_READ                     1352

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


// stream 协议
#define ERROR_STREAM_EOF                    3000
// flv 协议
#define ERROR_FLV_PROBE                     3101
#define ERROR_FLV_BUFFER_OVERFLOW           3102



#endif // SPS_ERROR_HPP

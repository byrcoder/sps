#ifndef SPS_TYPEDEF_HPP
#define SPS_TYPEDEF_HPP

#include <list>
#include <set>

#ifndef __unused

#define __unused __attribute__((unused))

#endif

typedef int error_t;
typedef unsigned long long utime_t;

#define SUCCESS 0L

#define ERROR_THREAD_DISPOSED              20
#define ERROR_THREAD_STARTED               21
#define ERROR_ST_CREATE_CYCLE_THREAD       22
#define ERROR_THREAD_TERMINATED            23
#define ERROR_THREAD_INTERRUPED            24

#define ERROR_CO_CREATE 100


// socket 操作错误吗 1000～1200 tcp错误
#define ERROR_SOCKET_CREATE                 1000
#define ERROR_SOCKET_SETREUSE               1001
#define ERROR_SOCKET_BIND                   1002
#define ERROR_SOCKET_LISTEN                 1003
#define ERROR_SOCKET_CLOSED                 1004
#define ERROR_SOCKET_GET_PEER_NAME          1005
#define ERROR_SOCKET_GET_PEER_IP            1006
#define ERROR_SOCKET_READ                   1007
#define ERROR_SOCKET_READ_FULLY             1008
#define ERROR_SOCKET_WRITE                  1009
#define ERROR_SOCKET_WAIT                   1010
#define ERROR_SOCKET_TIMEOUT                1011
#define ERROR_SOCKET_CONNECT                1012

#define ERROR_SOCKET_EINTR                  1065

// st 调用错误吗
#define ERROR_ST_OPEN_SOCKET                1200
#define ERROR_ST_CONNECT                    1201

// socket 错误码 1300～1499 srt错误
#define ERROR_SRT_SOCKET_CREATE             1300
#define ERROR_SRT_SOCKET_CONNECT            1301
#define ERROR_SRT_SOCKET_SEND               1302
#define ERROR_SRT_SOCKET_RECV               1303
#define ERROR_SRT_SOCKET_TIMEOUT            1304
#define ERROR_SRT_SOCKET_CLOSED             1305
#define ERROR_SRT_SOCKET_BROKEN             1306
#define ERROR_SRT_SOCKET_CLOSING            1307
#define ERROR_SRT_SOCKET_NOTEXITS           1308
#define ERROR_SRT_SOCKET_STATUS_OK          1309
#define ERROR_SRT_SOCKET_BIND               1310
#define ERROR_SRT_SOCKET_LISTEN             1311
#define ERROR_SRT_SOCKET_UNKNOWN            1320


// DNS 等解析错误
#define ERROR_SYSTEM_IP_INVALID             1500


// PROXY 业务配置错误
#define ERROR_PROXY_CONFIG_ERROR           1600
#define ERROR_PROXY_ARGS                   1601
#define ERROR_PROXY_ROUTE_FAILED           1602

// SYSTEM 内存
#define ERROR_MEM_OVERFLOW                 1702


#define ERROR_PROXY_PROTOCOL_HEADER_OVERFLOW      1800
#define ERROR_PROXY_PROTOCOL_HEADER_PARSE_FAILED  1801

#define ERROR_PROTOCOL_NOT_DEFINE                 1820

// FAST OPEN 不支持错误吗
#define ERROR_NOT_SUPPORT_TCP_FAST_OPEN           1900
#define ERROR_NOT_SUPPORT_SRT_FAST_OPEN           1901

#define ERROR_MEM_IO_FAILED  2000


#define ERROR_PROTOCOL_FLV  3000
#define ERROR_PROTOCOL_FLV_BUFFER 3001

#define ERROR_URL_RSP_NOT_INVALID 10000
#define ERROR_URL_PROTOCOL_EOF    10001


template<class T>
class SingleInstance {
 public:
    static T& get_instance() {
        static T obj;
        return obj;
    }
};

template<class T>
class Single {
 public:
    friend class SingleInstance<T>;
 protected:
    virtual ~Single() = default;
    Single() = default;
 private:
    Single(Single& ) = default;
};

template<class T, class S>
class Registers : public Single<T> {
 public:
    void reg(const S& obj) {
        objs.insert(obj);

    }

    void cancel(const S& obj) {
        objs.erase(obj);
    }

    std::set<S>& refs() {
        return objs;
    }
 protected:
    std::set<S> objs;
};

template<class T, class S>
class FifoRegisters : public Single<T> {
 public:
    void reg(const S& obj) {
        objs.push_back(obj);
    }

    void cancel(const S& obj) {
        auto it = std::find(objs.begin(), objs.end(), obj);
        if (it != objs.end()) {
            objs.erase(it);
        }
    }

    std::list<S>& refs() {
        return objs;
    }
 protected:
    std::list<S> objs;
};

#endif // SPS_TYPEDEF_HPP

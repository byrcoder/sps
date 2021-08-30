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

#include <sps_io_socket.hpp>

#include <memory>

#include <sps_log.hpp>
#include <sps_st_io_srt.hpp>
#include <sps_st_io_tcp.hpp>

namespace sps {

const char* get_transport_name(Transport t) {
    switch (t) {
        case TCP: return "tcp";
#ifndef SRT_DISABLED
        case SRT: return "srt";
#endif
        case QUIC: return "quic";
        case FILE: return "file";
        default: return "unknown";
    }
}

PSocket ClientSocketFactory::create_ss(
    Transport transport,
    const std::string &ip,
    int port, utime_t tm) {
    error_t ret = SUCCESS;

    switch (transport) {
        case Transport::TCP: {
            st_netfd_t fd;

            if ((ret = st_tcp_connect(ip, port, tm, &fd)) != SUCCESS) {
                sp_error("Failed tcp connect %s:%d, tm:%llu, ret:%d",
                         ip.c_str(), port, tm, ret);
                return nullptr;
            }

            return std::make_shared<Socket>(
                    std::make_shared<StTcpSocket>(fd), ip, port);
        }
        case Transport::SRT: {
            auto fd = st_srt_create_fd();

            if (fd == SRT_INVALID_SOCK) {
                sp_error("Failed create srt fd connect %s:%d, tm:%llu, ret:%d",
                         ip.c_str(), port, tm, ret);
                return nullptr;
            }

            if ((ret = st_srt_connect(fd, ip, port, tm)) != SUCCESS) {
                st_srt_close(fd);
                sp_error("Failed srt connect %s:%d, tm:%llu, ret:%d",
                         ip.c_str(), port, tm, ret);
                return nullptr;
            }

            return  std::make_shared<Socket>(
                    std::make_shared<StSrtSocket>(fd),
                    ip, port);
        }
        default:
            return nullptr;
    }
}

PIServerSocket ServerSocketFactory::create_ss(Transport transport) {
    switch (transport) {
        case Transport::TCP:
            return std::make_shared<StTcpServerSocket>();
        case Transport::SRT:
            return std::make_shared<StSrtServerSocket>();
        default:
            return nullptr;
    }
}

}  // namespace sps

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

#ifndef SPS_NET_MEMIO_HPP
#define SPS_NET_MEMIO_HPP

#include <sps_io.hpp>
#include <sps_typedef.hpp>

namespace sps {

class MemoryReaderWriter : public IReaderWriter {
 public:
    explicit MemoryReaderWriter(char *buf = nullptr, int len = 0);

 public:
    void init(char* buf, int len);

 public:
     void    set_recv_timeout(utime_t tm) override;
     utime_t get_recv_timeout() override;
     bool    seekable()  override;

    error_t read_fully(void* buf, size_t size, ssize_t* nread) override;
    error_t read(void* buf, size_t size, size_t& nread) override;
    error_t read_line(std::string &line) override;
    int     cur_line_num() override;

 public:
    void set_send_timeout(utime_t tm) override;
    utime_t get_send_timeout() override;

 public:
    error_t write(void* buf, size_t size) override;

 private:
    char*    buf;
    uint32_t len;
    uint32_t pos;
    int      line_num;
};

}

#endif  // SPS_NET_MEMIO_HPP

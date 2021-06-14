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

#ifndef SPS_NET_IO_HPP
#define SPS_NET_IO_HPP

#include <memory>

#include <sps_typedef.hpp>

class IReader {
 public:
    virtual ~IReader() = default;

 public:
    virtual void    set_recv_timeout(utime_t tm) = 0;
    virtual utime_t get_recv_timeout() = 0;
    virtual bool    seekable()  { return false; }

 public:
    /**
     * 读满数据
     */
    virtual error_t read_fully(void* buf, size_t size, ssize_t* nread = nullptr) = 0;

    /**
     * 尽量读数据
     */
    virtual error_t read(void* buf, size_t size, size_t& nread) = 0;

    virtual error_t read_line(std::string& line) { throw "not impl"; }

    virtual error_t cur_line_num()  { return 0; }
};

class IWriter {
 public:
    virtual ~IWriter() = default;

 public:
    virtual void set_send_timeout(utime_t tm) = 0;
    virtual utime_t get_send_timeout() = 0;

 public:
    virtual error_t write(void* buf, size_t size) = 0;
};

class IReaderWriter : virtual public IReader, virtual public IWriter {
 public:
    IReaderWriter() = default;
    ~IReaderWriter() override = default;
};

typedef std::shared_ptr<IReader> PIReader;
typedef std::shared_ptr<IReaderWriter> PIReaderWriter;

#endif  // SPS_NET_IO_HPP

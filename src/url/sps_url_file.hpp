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

#ifndef SPS_NET_FILE_HPP
#define SPS_NET_FILE_HPP

#include <fstream>
#include <memory>
#include <string>

#include <sps_url_protocol.hpp>

namespace sps {

class FileURLProtocol : public IURLProtocol {
 public:
    explicit FileURLProtocol(bool trunc = false, bool append = false);
    ~FileURLProtocol() override;

 public:
    error_t open(PRequestUrl& url, Transport) override;
    error_t open(const std::string& filename) override;

    PResponse response() override;

 public:
    void    close();

 public:
    void    set_recv_timeout(utime_t tm) override {  }
    utime_t get_recv_timeout()  override { return 0; }
    bool    seekable()  override { return true; }

    /**
    * 读满数据
    */
    error_t read_fully(void* buf, size_t size, ssize_t* nread) override;

    /**
     * 尽量读数据
     */
    error_t read(void* buf, size_t size, size_t& nread) override;

    error_t read_line(std::string& line) override;

    int     cur_line_num() override;

 public:
    error_t write(void* buf, size_t size) override;
    void    set_send_timeout(utime_t tm) override  {             }
    utime_t get_send_timeout() override            { return 0;   }

 private:
    std::string   filename;
    std::fstream  fh;
    int           line_num;

    bool          trunc;
    bool          append;
    bool          create;
};

class FileURLProtocolFactory : public IURLProtocolFactory {
 public:
    FileURLProtocolFactory();

    PIURLProtocol create(PRequestUrl url) override;
};
typedef std::shared_ptr<FileURLProtocolFactory> PFileUrlProtocolFactory;

}  // namespace sps

#endif  // SPS_NET_FILE_HPP

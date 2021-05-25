#ifndef SPS_MEM_HPP
#define SPS_MEM_HPP

#include "net/io.hpp"
#include "typedef.hpp"

namespace sps {

class MemReaderWriter : public IReaderWriter {

 public:
    MemReaderWriter(char *buf = nullptr, int len = 0);

 public:
    void init(char* buf, int len);

 public:
     void    set_recv_timeout(utime_t tm) override;
     utime_t get_recv_timeout() override;
     bool    seekable()  override;

    error_t read_fully(void* buf, size_t size, ssize_t* nread) override;
    error_t read(void* buf, size_t size, size_t& nread) override;

 public:
    void set_send_timeout(utime_t tm) override;
    utime_t get_send_timeout() override;

 public:
    error_t write(void* buf, size_t size) override;

 private:
    char*    buf;
    uint32_t len;
    uint32_t pos;
};

}

#endif // SPS_MEM_HPP

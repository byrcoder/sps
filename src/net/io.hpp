#ifndef SPS_NET_IO_HPP_
#define SPS_NET_IO_HPP_

#include <memory>
#include "typedef.hpp"

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
    virtual error_t read_fully(void* buf, size_t size, ssize_t* nread) = 0;

    /**
     * 尽量读数据
     */
    virtual error_t read(void* buf, size_t size, size_t& nread) = 0;
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

#endif  // SPS_NET_IO_HPP_

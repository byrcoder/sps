#ifndef SPS_CACHE_BUFFER_HPP
#define SPS_CACHE_BUFFER_HPP

#include <memory>

namespace sps {

class IBuffer {
 public:
    virtual ~IBuffer() = default;

 public:
    virtual char *buffer() = 0;

    virtual int size() = 0;
};

typedef std::shared_ptr<IBuffer> PIBuffer;

class CharBuffer;
typedef std::shared_ptr<CharBuffer> PCharBuffer;

class CharBuffer : public IBuffer {
 public:
    static PCharBuffer copy(const char* buf, int len);
    // static  __unused PCharBuffer nocopy(char* buf, int len);

 public:
    CharBuffer(const char* buf, int len);
    ~CharBuffer();

 public:
    char* buffer() override      { return buf; }
    int   size() override        { return len; }

 private:
    char* buf = nullptr;
    int   len = 0;
};

}

#endif  // SPS_CACHE_BUFFER_HPP

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

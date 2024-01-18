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

#ifndef SPS_LOG_HPP
#define SPS_LOG_HPP

#include <cerrno>
#include <cstdio>
#include <cstring>

#include <sps_auto_header.hpp>

#define __FILENAME__ (__builtin_strrchr(__FILE__, '/') ? \
    __builtin_strrchr(__FILE__, '/') + 1 : __FILE__)

namespace sps {

enum LOG_LEVEL {
    DEBUG,
    INFO,
    TRACE,
    WARN,
    ERROR,
};

class ILog {
 public:
    virtual void printf(LOG_LEVEL lev, const char*msg, ...) = 0;
};

class ConsoleLog : public ILog {
 public:
    void printf(LOG_LEVEL lev, const char* fmt, ...) override;

 private:
    char buf[4096];
};

extern int get_context_id();

extern ILog* log;

};


#define sp_error(msg , ...) sps::log->printf(sps::LOG_LEVEL::ERROR, "[%s:%d] [%d] " msg " (%d %s)\r\n", \
    __FILENAME__, __LINE__, sps::get_context_id(),  ##__VA_ARGS__, errno, strerror(errno))

#define sp_warn(msg , ...)  sps::log->printf(sps::LOG_LEVEL::WARN, "[%s:%d] [%d] " msg "\r\n", \
    __FILENAME__, __LINE__, sps::get_context_id(), ##__VA_ARGS__)

#define sp_trace(msg, ...)  sps::log->printf(sps::LOG_LEVEL::TRACE, "[%s:%d] [%d] " msg "\r\n", \
    __FILENAME__, __LINE__, sps::get_context_id(), ##__VA_ARGS__)

#define sp_info(msg, ...)   sps::log->printf(sps::LOG_LEVEL::INFO, "[%s:%d] [%d] " msg "\r\n", \
    __FILENAME__, __LINE__, sps::get_context_id(), ##__VA_ARGS__)

#define sp_append_start(msg, ...)  \
     sps::log->printf(sps::LOG_LEVEL::INFO, "[%s:%d] " msg " ", __FILENAME__, __LINE__,  ##__VA_ARGS__)

#define sp_append(msg, ...)         sps::log->printf(sps::LOG_LEVEL::INFO, msg " ",  ##__VA_ARGS__)
#define sp_append_end()             sps::log->printf(sps::LOG_LEVEL::INFO,"\r\n");


#ifdef SPS_HEAVY_LOG
#define sp_debug(msg, ...)  \
    sps::log->printf(sps::LOG_LEVEL::DEBUG, "[%s:%d] " msg "\r\n", __FILENAME__, __LINE__,  ##__VA_ARGS__)
#else
#define sp_debug(msg, ...)
#endif

#endif  // SPS_LOG_HPP

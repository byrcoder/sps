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

#include <stdio.h>

#include <sps_auto_header.hpp>

#define __FILENAME__ (__builtin_strrchr(__FILE__, '/') ? __builtin_strrchr(__FILE__, '/') + 1 : __FILE__)

#define sp_error(msg , ...) printf("[%s:%d] " msg "\r\n", __FILENAME__, __LINE__,  ##__VA_ARGS__)
#define sp_warn(msg , ...)  printf("[%s:%d] " msg "\r\n", __FILENAME__, __LINE__,  ##__VA_ARGS__)
#define sp_trace(msg, ...)  printf("[%s:%d] " msg "\r\n", __FILENAME__, __LINE__,  ##__VA_ARGS__)
#define sp_info(msg, ...)   printf("[%s:%d] " msg "\r\n", __FILENAME__, __LINE__,  ##__VA_ARGS__)

#define sp_append_start(msg, ...)   printf("[%s:%d] " msg " ", __FILENAME__, __LINE__,  ##__VA_ARGS__)
#define sp_append(msg, ...)         printf(msg " ",  ##__VA_ARGS__)
#define sp_append_end()             printf("\r\n");


#ifdef SPS_HEAVY_LOG
#define sp_debug(msg, ...)  printf("[%s:%d] " msg "\r\n", __FILENAME__, __LINE__,  ##__VA_ARGS__)
#else
#define sp_debug(msg, ...)
#endif

#endif  // SPS_LOG_HPP

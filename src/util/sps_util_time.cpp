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

//
// Created by byrcoder on 2021/9/30.
//

#include <sps_util_time.hpp>

#include <time.h>
#include <sys/time.h>

#include <string>
#include <sstream>
#include <iomanip>

namespace sps {

utime_t update_time() {
    timeval now;

    if (gettimeofday(&now, NULL) < 0) {
        return 0;
    }

    return now.tv_sec * 1000 + now.tv_usec / 1000;
}

utime_t get_time() {
    return update_time();
}

std::string to_hex(const char* msg, int len) {
    std::stringstream ss;
    ss << "hex: 0x";
    for (int i = 0; i < len; ++i) {
        ss << std::hex << (int) msg[i] << " ";
    }

    return ss.str();
}

}
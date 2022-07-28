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
// Created by byrcoder on 2022/3/9.
//

#include <gtest/gtest.h>
#include <sps_log.hpp>

using namespace std;

struct Int32 {
    int32_t a32;
};

struct Int64 {
    int32_t a32;
    int32_t b32;
};

struct Int96 {
    int32_t a32;
    int32_t b32;
    int32_t c32;
};
#if 0
GTEST_TEST(THREAD, ATOMIC) {
    std::atomic<int> i32(1);
    std::atomic<bool> b(false);
    std::atomic<int64_t> i64;

    std::atomic<Int32> si32;
    std::atomic<Int64> si64;
    std::atomic<Int96> si96;

    EXPECT_TRUE(i32.is_lock_free());
    EXPECT_TRUE(b.is_lock_free());
    EXPECT_TRUE(i64.is_lock_free());
    EXPECT_TRUE(si32.is_lock_free());
    EXPECT_TRUE(si64.is_lock_free());
    // EXPECT_TRUE(si96.is_lock_free());  compile error

}
#endif

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
// Created by byrcoder on 2021/7/27.
//

#include <gtest/gtest.h>
#include <sps_io_bits.hpp>
#include <sps_log.hpp>

using namespace sps;

GTEST_TEST(IOBITS, READ) {
    uint8_t buf[] = {
            0XD7, // 1101 0111        // 0
            0X02, 0X03,               // 1
            0X04, 0X05, 0X06,         // 3
            0X07, 0X08, 0X09,         // 6
            0XA0, 0XB0, 0XC0, 0XD0,   // 9
    };

    for (int i = 0; i < 2; ++i) {
        BitContext bits(buf, sizeof(buf));

        EXPECT_TRUE(bits.size_bits() == sizeof(buf) * 8);

        EXPECT_TRUE(bits.read_bits(1) == 1);
        EXPECT_TRUE(bits.read_bits(2) == 2);
        EXPECT_TRUE(bits.read_bits(5) == 0x17); // 1 byte

        EXPECT_TRUE(bits.read_bits(9) == 0x04); // 2~3byte
        EXPECT_TRUE(bits.read_bits(7) == 0x03);


        if (i == 0) {
            EXPECT_TRUE(bits.read_bits(8) == 0x04);  // 0x04
            EXPECT_TRUE(bits.read_bits(16) == 0x0506);
            EXPECT_TRUE(bits.read_bits(24) == 0x070809);
            EXPECT_TRUE(bits.read_bits(32) == 0xA0B0C0D0);
        } else {
            EXPECT_TRUE(bits.read_bits(15) == 0x0202); // 0x04 0x05 -> 0x 0100 0000  0000 010  1
            EXPECT_TRUE(bits.read_bits(17) == 0x010607); // 0x 06  07

            EXPECT_TRUE(bits.read_bits(4) == 0x00);
            EXPECT_TRUE(bits.read_bits(24) == 0x809a0b);  // left 0xB 0
            EXPECT_TRUE(bits.read_bits(20) == 0x0C0D0);
        }

        EXPECT_TRUE(bits.size_bits() == 0);
    }

    {
        BitContext bits(buf, sizeof(buf));
        uint8_t cp[sizeof(buf)];

        bits.read_bytes(cp, 1);
        bits.read_bytes(cp + 1, 2);
        bits.read_bytes(cp + 3, 3);
        bits.read_bytes(cp + 6, 3);
        bits.read_bytes(cp + 9, 4);
        EXPECT_TRUE(bits.size_bits() == 0);

        EXPECT_TRUE(memcmp(cp, buf, sizeof(buf)) == 0);

    }

}
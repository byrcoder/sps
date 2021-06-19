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
// Created by byrcoder on 2021/6/18.
//

#include <gtest/gtest.h>
#include <sps_io_bytes.hpp>
#include <sps_io_memory.hpp>
#include <sps_log.hpp>

GTEST_TEST(IOBYTES, READ) {
    uint8_t buf[] = {
            0X01,
            0X02, 0X03,
            0X04, 0X05, 0X06,
            0X07, 0X08, 0X09, 0XA0,
            'a', 'b', 'c', 'd', 'e', 'f',
            '\0',
    };

    PIReader rw = std::make_shared<sps::MemoryReaderWriter>((char*) buf, sizeof(buf));

    auto buffer = std::make_unique<sps::AVBuffer>(7, true);
    sps::SpsBytesReader bytes(rw, buffer);

    sp_info("total bytes: %s", (char*) buf);

    {

        uint32_t n = 0;
        EXPECT_TRUE(bytes.acquire(1) == SUCCESS);
        EXPECT_TRUE((n=bytes.read_int8())== 0X01);
        sp_info("read 1-byte 0X01 = %X ", n);
    }

    {
        uint32_t n = 0;
        EXPECT_TRUE(bytes.acquire(2) == SUCCESS);
        EXPECT_TRUE((n=bytes.read_int16()) == 0X0203);
        sp_info("read 2-byte 0X0203 = %X", n);
    }

    {
        uint32_t n = 0;
        EXPECT_TRUE(bytes.acquire(3) == SUCCESS);
        EXPECT_TRUE((n=bytes.read_int24()) == 0X040506);
        sp_info("read 3-byte 0X040506 = %X", n);
    }

   {
       uint32_t n = 0;
       EXPECT_TRUE(bytes.acquire(4) == SUCCESS);
       EXPECT_TRUE((n=bytes.read_int32()) == 0X070809A0);
       sp_info("read 4-byte 0X070809A0 = %X", n);
   }

    {
        int n = 0;
        uint8_t buf[7] = {0};
        int     len = sizeof(buf) - 1; // sizeof(char) = 8, sizeof(uint8_t) = 7
        EXPECT_TRUE(bytes.acquire(len) == SUCCESS);
        bytes.read_bytes(buf, len);
        std::string tmp_buf((char*)buf, len);
        EXPECT_TRUE(tmp_buf == "abcdef");
        sp_info("read 6-byte abcef = %s", tmp_buf.c_str());
        sp_info("io %lu, tmp buf %s, %s",  sizeof(buf), tmp_buf.c_str(), (char*) buffer->buffer());
    }
}

GTEST_TEST(IOBYTES, REWIND) {

    std::string tmp(1024, '1') ;

    for (int i = 0 ; i < tmp.size(); ++i) {
        tmp[i] = (char) rand() % 127  + 1;
    }

    PIReader rw = std::make_shared<sps::MemoryReaderWriter>((char*) tmp.c_str(), tmp.size());
    auto buffer = std::make_unique<sps::AVBuffer>(7, true);
    sps::SpsBytesReader bytes(rw, buffer);

    for (int i = 0; i < tmp.size(); ) {
        int n = rand() % 7 + 1;
        int len = std::min(n, (int) tmp.size() - i);
        char* buf = new char[len+1] {0};

        EXPECT_TRUE(bytes.acquire(len) == SUCCESS);

        bytes.read_bytes((uint8_t*) buf, len);
        EXPECT_TRUE(memcmp(tmp.c_str() + i, buf, len) == 0);

        i += len;
        delete[] buf;
    }
}

GTEST_TEST(IOBYTES, NOREWIND) {

    std::string tmp(1024, '1') ;

    for (int i = 0 ; i < tmp.size(); ++i) {
        tmp[i] = (uint8_t) rand() % 255 + 1;
    }

    PIReader rw = std::make_shared<sps::MemoryReaderWriter>((char*) tmp.c_str(), tmp.size());
    auto buffer = std::make_unique<sps::AVBuffer>(tmp.size(), true);
    sps::SpsBytesReader bytes(rw, buffer);

    char* buf = new char[tmp.size()] {0};

    for (int i = 0; i < tmp.size(); ) {
        int n = rand() % 7 + 1;
        int len = std::min(n, (int) tmp.size() - i);

        EXPECT_TRUE(bytes.acquire(len) == SUCCESS);
        bytes.read_bytes((uint8_t*) buf, len);
        EXPECT_TRUE(memcmp(tmp.c_str() + i, buf, len) == 0);
        i += len;
    }
    delete[] buf;


    EXPECT_TRUE( memcmp((char*)tmp.c_str(), (char*) buffer->buffer(), tmp.size()) == 0);
    for (int i = 0; i < tmp.size(); ++i) {
        sp_info("%d: %u, %u", i, tmp[i], *(buffer->buffer()+i));
    }

    EXPECT_TRUE(bytes.acquire(1) != SUCCESS);
}

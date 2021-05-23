#include <http/parser.hpp>
#include <gtest/gtest.h>
extern "C" {
#include <public.h>
}

GTEST_TEST(HTTP, CREATE) {
    sps::HttpParser parser;
    const char* buf = "HTTP/1.1 200 OK\r\n"
                      "Date: Tue, 04 Aug 2009 07:59:32 GMT\r\n"
                      "Server: Apache\r\n"
                      "Content-Type: text/xml; charset=utf-8\r\n"
                      "Connection: close\r\n\r\n";
    int len = strlen(buf);
    auto x = parser.parse_header(buf, len, sps::BOTH);

    EXPECT_TRUE(x == len);
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    st_init();
    return RUN_ALL_TESTS();
}
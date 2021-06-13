#include <app/http/sps_http_parser.hpp>
#include <gtest/gtest.h>
#include <net/sps_net_memio.hpp>
#include <log/sps_log.hpp>
extern "C" {
#include <public.h>
}

GTEST_TEST(HTTP_RESPONSE, CREATE) {
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

GTEST_TEST(HTTP_REQUEST, CREATE) {
    sps::HttpParser parser;
    const char* buf = "GET /test/demo_form.php?xxx=a&yy=b&zz=c HTTP/1.1\r\n"
                      "Host: runoob.com:28\r\n"
                      "Content-Length: 20\r\n"
                      "\r\n"
                      "--------------------------------------";

    int len = strlen(buf);
    auto x = len; // parser.parse_header(buf, len, sps::BOTH);

    auto m = std::make_shared<sps::MemoryReaderWriter>((char *) buf, strlen(buf));
    auto y = parser.parse_header(m, sps::BOTH);

    sp_info("%d, %d, %d", len, x, y);
    // EXPECT_TRUE(x == len);
    EXPECT_TRUE(y == (strstr(buf, "--") - buf));
}
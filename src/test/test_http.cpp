#include <http/parser.hpp>
#include <http/client.hpp>
#include <gtest/gtest.h>
#include <net/mem.hpp>
#include <log/logger.hpp>
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

    auto m = std::make_shared<sps::MemReaderWriter>((char *) buf, strlen(buf));
    auto y = parser.parse_header(m, sps::BOTH);

    sp_info("%d, %d, %d", len, x, y);
    // EXPECT_TRUE(x == len);
    EXPECT_TRUE(y == len);
}

GTEST_TEST(HTTP_CLIENT, GET) {
    auto client = sps::HttpClient::create_http_create();

    EXPECT_TRUE(client->init("www.baidu.com") == SUCCESS);

    EXPECT_TRUE(client->get("/") == SUCCESS);

    EXPECT_TRUE(client->status_code() == 200);
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    st_init();
    return RUN_ALL_TESTS();
}
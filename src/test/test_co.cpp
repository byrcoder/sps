#include <gtest/gtest.h>
#include <st/net/tcp.hpp>

GTEST_TEST(CO, CREATE) {
    EXPECT_TRUE(sps::st_tcp_open_fd(100) == nullptr);
    st_sleep(1);
    EXPECT_TRUE(sps::st_tcp_open_fd(200) == nullptr);
}
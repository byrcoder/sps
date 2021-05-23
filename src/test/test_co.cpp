#include <gtest/gtest.h>
#include <co/st/tcp.hpp>

GTEST_TEST(CO, CREATE) {
EXPECT_TRUE(st_tcp_open_fd(100) == nullptr);
}
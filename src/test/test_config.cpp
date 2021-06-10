#include <gtest/gtest.h>

#include <app/config/conf_upstream.hpp>

#include <net/net_memio.hpp>

using namespace sps;

GTEST_TEST(CONFIG, UPSTREAM) {

    std::string conf = "\n"
                           "    server 127.0.0.1:8080;\n"
                           "    keepalive 80;\n"
                           "\n"
                           "}";

    UpstreamConfigInstantFactory factory;
    ConfUpstream upstream;

    auto instant = factory.create();

    auto rd = std::make_shared<MemoryReaderWriter>((char*)conf.c_str(), conf.size());

    int line = 0;

    EXPECT_TRUE(instant->set_opts(rd, line) == SUCCESS);

}
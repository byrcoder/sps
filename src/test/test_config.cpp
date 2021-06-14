#include <gtest/gtest.h>

#include <app/upstream/sps_upstream_module.hpp>

#include <net/sps_net_memio.hpp>

#include <log/sps_log.hpp>

using namespace sps;

GTEST_TEST(CONFIG, UPSTREAM) {

    std::string conf = "\n"
                           "    server 127.0.0.1:8080;\n"
                           "    keepalive 80;\n"
                           "\n"
                           "}";

    UpStreamModuleFactory factory;

    auto module = factory.create("upstream", "x1", nullptr);

    auto rd = std::make_shared<MemoryReaderWriter>((char*)conf.c_str(), conf.size());

    int line = 0;

    EXPECT_TRUE(module->init_conf(rd) == SUCCESS);

    const ConfCtx* vconf = module->conf.get();

    const UpStreamConfCtx* up = static_cast<const UpStreamConfCtx*>(vconf);

    EXPECT_TRUE(up != nullptr);

    EXPECT_TRUE(up->server == "127.0.0.1:8080");

    EXPECT_TRUE(up->keepalive == 80);

    sp_info("%s, %d", up->server.c_str(), up->keepalive);
}
#include <cache/cache.hpp>
#include <cache/buffer.hpp>
#include <sync/sync.hpp>
#include <log/logger.hpp>

#include <string>
#include <list>

#include <gtest/gtest.h>

GTEST_TEST(SYNC, CREATE) {
    EXPECT_TRUE(true);
}

GTEST_TEST(CACHE, CREATE) {
    auto c =  std::make_shared<sps::InfiniteCache>();
    auto cs1 = std::make_shared<sps::CacheStream>();

    c->put("cs1", cs1);

    auto cs = c->get("cs1");
    EXPECT_TRUE(cs1.get() == cs.get());

    sp_trace("%p == %p", cs1.get(), cs.get());

    auto sub_cs2 = std::make_shared<sps::CacheStream>();
    auto sub_cs3 = std::make_shared<sps::CacheStream>();

    cs1->sub(sub_cs2);
    cs1->sub(sub_cs3);

    std::string message1 = "message1";
    cs1->put(sps::CharBuffer::copy(message1.c_str(), message1.size()));

    std::string message2 = "message22";
    cs1->put(sps::CharBuffer::copy(message1.c_str(), message1.size()));

    std::list<sps::PIBuffer> dumps;
    sub_cs2->dump(dumps);
    EXPECT_TRUE(dumps.size() == 2);

    sp_trace("char length: %d=%d=%d", cs1->size(), sub_cs2->size(), sub_cs3->size());


    for (auto l : dumps) {
        sp_trace("message: %s.", std::string(l->buffer(), l->size()).c_str());
    }
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    st_init();
    return RUN_ALL_TESTS();
}
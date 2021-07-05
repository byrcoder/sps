#include <sps_cache.hpp>
#include <sps_cache_buffer.hpp>
#include <sps_sync.hpp>
#include <sps_log.hpp>

#include <string>
#include <list>

#include <gtest/gtest.h>

using namespace sps;

GTEST_TEST(SYNC, CREATE) {
    EXPECT_TRUE(true);
}

GTEST_TEST(CACHE, CREATE) {
    auto c =  std::make_shared<sps::InfiniteCache<PIBuffer>>();
    auto cs1 = std::make_shared<sps::CacheStream<PIBuffer>>();

    c->put("cs1", cs1);

    auto cs = c->get("cs1");
    EXPECT_TRUE(cs1.get() == cs.get());

    sp_trace("%p == %p", cs1.get(), cs.get());

    auto sub_cs2 = std::make_shared<sps::CacheStream<PIBuffer>>();
    auto sub_cs3 = std::make_shared<sps::CacheStream<PIBuffer>>();

    cs1->sub(sub_cs2);
    cs1->sub(sub_cs3);

    std::string message1 = "message1";
    cs1->put(sps::CharBuffer::deep_copy((uint8_t*) message1.c_str(), message1.size()));

    std::string message2 = "message22";
    cs1->put(sps::CharBuffer::deep_copy((uint8_t*) message2.c_str(), message2.size()));

    std::list<sps::PIBuffer> dumps;
    sub_cs2->dump(dumps, false);
    EXPECT_TRUE(dumps.size() == 2);
    sp_trace("1. char length: %d=%d=%d", cs1->size(), sub_cs2->size(), sub_cs3->size());
    for (auto l : dumps) {
        sp_trace("message: %s.", std::string((char*) l->buffer(), l->size()).c_str());
    }

    std::list<sps::PIBuffer> dumps2;
    sub_cs2->dump(dumps2, true);
    EXPECT_TRUE(dumps2.size() == 2);
    sp_trace("2. char length: %d=%d=%d", cs1->size(), sub_cs2->size(), sub_cs3->size());
    for (auto l : dumps2) {
        sp_trace("message: %s.", std::string((char*) l->buffer(), l->size()).c_str());
    }

    std::list<sps::PIBuffer> dumps3;
    sub_cs2->dump(dumps3, true);
    EXPECT_TRUE(dumps3.size() == 0);
    sp_trace("3. char length: %d=%d=%d", cs1->size(), sub_cs2->size(), sub_cs3->size());
    for (auto l : dumps3) {
        sp_trace("message: %s.", std::string((char*) l->buffer(), l->size()).c_str());
    }
}
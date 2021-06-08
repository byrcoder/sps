#include <app/url/url.hpp>
#include <app/url/url_protocol.hpp>
#include <app/url/url_http.hpp>

#include <typedef.hpp>

#include <gtest/gtest.h>
#include <net/net_memio.hpp>
#include <log/log_logger.hpp>
extern "C" {
#include <public.h>
}

auto& protocols = SingleInstance<sps::UrlProtocol>::get_instance();

GTEST_TEST(URL_REQUEST, CREATE) {
    const std::string url = "http://www.baidu.com/?a=b&c=2";
    auto p = protocols.create(url);

    EXPECT_TRUE(p != nullptr);

    EXPECT_TRUE(p->open(url, sps::DEFAULT) == SUCCESS);

    char buf[1024] = { 0 };
    size_t nread = 0;
    auto ret = p->read(buf, 1023, nread);
    EXPECT_TRUE(ret == SUCCESS);

    if (ret != SUCCESS) return;
    sp_info("%s.", buf);
}

int main(int argc, char **argv) {
    protocols.reg(
            std::make_shared<sps::HttpProtocolFactory>()
            );
    testing::InitGoogleTest(&argc, argv);
    st_init();
    return RUN_ALL_TESTS();
}
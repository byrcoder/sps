#include <app/url/sps_url.hpp>
#include <app/url/sps_url_protocol.hpp>
#include <app/url/sps_url_http.hpp>

#include <sps_typedef.hpp>

#include <gtest/gtest.h>
#include <net/sps_net_memio.hpp>
#include <log/sps_log.hpp>
extern "C" {
#include <public.h>
}


GTEST_TEST(URL_REQUEST, CREATE) {
    auto& protocols = SingleInstance<sps::UrlProtocol>::get_instance();
    const std::string url = "http://www.baidu.com/?a=b&c=2";
    auto p = protocols.create(url);

    error_t ret = SUCCESS;
    EXPECT_TRUE(p != nullptr);

    if (!p) return;
    EXPECT_TRUE((ret = p->open(url, sps::DEFAULT)) == SUCCESS);
    if (ret != SUCCESS) return;

    char buf[1024] = { 0 };
    size_t nread = 0;
    ret = p->read(buf, 1023, nread);
    EXPECT_TRUE(ret == SUCCESS);

    if (ret != SUCCESS) return;
    sp_info("%s.", buf);
}
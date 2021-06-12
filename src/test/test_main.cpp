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

int main(int argc, char **argv) {
    protocols.reg(
            std::make_shared<sps::HttpProtocolFactory>()
    );
    testing::InitGoogleTest(&argc, argv);
    st_init();
    return RUN_ALL_TESTS();
}
#include <sps_url.hpp>
#include <sps_url_protocol.hpp>
#include <sps_url_http.hpp>

#include <sps_typedef.hpp>

#include <gtest/gtest.h>
#include <sps_io_memory.hpp>
#include <sps_log.hpp>
#include <sps_install.hpp>
extern "C" {
#include <public.h>
}

auto& protocols = SingleInstance<sps::UrlProtocol>::get_instance();

const char* filename     = "media/third.flv";
const char* out_filename = "media/out.flv";

int main(int argc, char **argv) {

    sps::sps_once_install();
//    protocols.reg(
//            std::make_shared<sps::HttpURLProtocolFactory>()
//    );
    if (argc > 1) {
        filename = argv[1];
    }

    if (argc > 2) {
        out_filename = argv[2];
    }
    testing::InitGoogleTest(&argc, argv);
    st_init();
    return RUN_ALL_TESTS();
}
#include <co/co.hpp>
#include <public.h>
#include <server/http_server.hpp>
#include <log/logger.hpp>

int main(int argc, char* argv[]) {
    st_init();
    auto p = sps::ICoFactory::get_instance().start(nullptr);

    sp_trace("starting null:%d", p);

    auto http_server = std::make_shared<sps::HttpServer>();
    if (http_server->listen("127.0.0.1", 8000) != SUCCESS) {
        sp_error("Failed listen");
        return 0;
    }

    sp_error("Success listen");

    sps::ICoFactory::get_instance().start(http_server);
    st_sleep(-1);
    return 0;
}
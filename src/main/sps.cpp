#include <vector>

#include <co/co.hpp>

#include <app/http/http_phase_handler.hpp>
#include <app/http/http_server.hpp>

#include <log/log_logger.hpp>

#include <sync/sync.hpp>
#include <app/url/url_protocol.hpp>
#include <app/url/url_http.hpp>

std::vector<sps::PServer> servers;

error_t init_protocol() {
    auto& protocols = SingleInstance<sps::UrlProtocol>::get_instance();
    protocols.reg(
            std::make_shared<sps::HttpProtocolFactory>()
    );

    return SUCCESS;
}

error_t init_http() {

    auto& hp = SingleInstance<sps::HttpPhaseHandler>::get_instance();

    hp.reg(std::make_shared<sps::HttpParsePhaseHandler>());
    hp.reg(std::make_shared<sps::HttpProxyPhaseHandler>());
    // hp.reg(std::make_shared<sps::Http404PhaseHandler>());

    auto http_server = std::make_shared<sps::HttpServer>();
    if (http_server->listen("127.0.0.1", 8000) != SUCCESS) {
        sp_error("Failed http listen");
        return 0;
    }

    sp_error("Success http server listen");
    servers.push_back(http_server);

    return sps::ICoFactory::get_instance().start(http_server);
}

int main(int argc, char* argv[]) {
    auto ret = sps::ICoFactory::get_instance().init();

    if (ret != SUCCESS) {
        sp_error("Failed co init ret:%d", ret);
        return ret;
    }

    if ((ret = init_protocol()) != SUCCESS) {
        sp_error("Failed init protocol ret:%d", ret);
        return ret;
    }

    if ((ret = init_http()) != SUCCESS) {
        sp_error("Failed start http server ret:%d", ret);
        return ret;
    }

    SingleInstance<sps::Sleep>::get_instance().sleep(SLEEP_FOREVER);
    return 0;
}
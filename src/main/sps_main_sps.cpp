#include <vector>

#include <app/core/sps_core_config.hpp>

#include <app/http/sps_http_phase_handler.hpp>
#include <app/http/sps_http_server.hpp>

#include <app/server/sps_server_config.hpp>


#include <app/url/sps_url_protocol.hpp>
#include <app/url/sps_url_http.hpp>

#include <co/sps_co.hpp>


#include <log/sps_log.hpp>

#include <net/sps_net_file.hpp>

#include <sync/sps_sync.hpp>

#include <typedef.hpp>

extern sps::PIModuleFactory modules[];

std::vector<sps::PServer> servers;
sps::PCoreModule core_module;

error_t init_co() {
    return sps::ICoFactory::get_instance().init();
}

void init_modules() {
    int i = 0;
    while(modules[i]) {
        SingleInstance<sps::ModuleFactoryRegister>::get_instance()
            .reg(std::string(modules[i]->module), modules[i]);
        sp_info("register module %s", modules[i]->module.c_str());
        ++i;
    }

    sp_info("total module %d", i);
}

error_t init_protocol() {
    auto& protocols = SingleInstance<sps::UrlProtocol>::get_instance();
    protocols.reg(std::make_shared<sps::HttpProtocolFactory>());

    return SUCCESS;
}

error_t init_config(const char* filename) {
    error_t ret  = SUCCESS;
    auto    fr   = std::make_shared<sps::FileReader>(filename);

    core_module = std::dynamic_pointer_cast<sps::CoreModule>(SingleInstance<sps::ModuleFactoryRegister>::get_instance()
            .create("core", "",  nullptr));

    if ((ret = fr->initialize()) != SUCCESS) {
        sp_error("Failed open file %s", filename);
        return ret;
    }

    if ((ret = core_module->init_conf(fr)) != SUCCESS) {
        sp_error("Failed init config(%s), ret:%d", filename, ret);
        return ret;
    }

    return SUCCESS;
}

error_t run_http_server() {
    error_t ret = SUCCESS;
    for (auto& http : core_module->http_modules) {
        for (auto& server : http->server_modules) {
            auto server_conf = std::static_pointer_cast<sps::ServerConfCtx>(server->conf);
            auto hp          = std::make_shared<sps::HttpPhaseHandler>();

            hp->reg(std::make_shared<sps::HttpParsePhaseHandler>());
            hp->reg(std::make_shared<sps::HttpRouterPhaseHandler>(server));

            auto http_server = std::make_shared<sps::HttpServer>(hp);

            if (http_server->listen("", server_conf->listen_port) != SUCCESS) {
                sp_error("Failed http listen :%d", server_conf->listen_port);
                return 0;
            }
            sp_info("success http server listen %d", server_conf->listen_port);
            servers.push_back(http_server);

            ret = sps::ICoFactory::get_instance().start(http_server);
            if (ret != SUCCESS) {
                sp_error("Failed start http server ret:%d", ret);
                return ret;
            }
        }
    }

    return ret;
}

error_t init() {
    error_t ret = SUCCESS;

    if ((ret = init_co()) != SUCCESS) {
        sp_error("Failed co init ret:%d", ret);
        return ret;
    }

    if ((ret = init_protocol()) != SUCCESS) {
        sp_error("Failed init protocol ret:%d", ret);
        return ret;
    }

    return ret;
}

error_t run_servers() {
    error_t ret = SUCCESS;

    if ((ret = run_http_server()) != SUCCESS) {
        sp_error("Failed start http server ret:%d", ret);
        return ret;
    }

    return ret;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        sp_error("Argv must greater 1: ./sps sps.conf");
        return -1;
    }

    init_modules();

    auto ret = init_config(argv[1]);

    if (ret != SUCCESS) {
        return ret;
    }

    ret = init();

    if (ret != SUCCESS) {
        return ret;
    }

    if ((ret = run_servers()) != SUCCESS) {
        return ret;
    }

    SingleInstance<sps::Sleep>::get_instance().sleep(SLEEP_FOREVER);
    return 0;
}
#include <vector>

#include <app/config/conf_core.hpp>

#include <app/http/http_phase_handler.hpp>
#include <app/http/http_server.hpp>

#include <app/url/url_protocol.hpp>
#include <app/url/url_http.hpp>

#include <co/co.hpp>


#include <log/log_logger.hpp>

#include <net/net_file.hpp>

#include <sync/sync.hpp>

#include <typedef.hpp>

extern sps::PIModuleFactory modules[];

std::vector<sps::PServer> servers;
sps::PIModule core_module;

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

    core_module = SingleInstance<sps::ModuleFactoryRegister>::get_instance()
            .create("core", "",  nullptr);

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

    if ((ret = init_http()) != SUCCESS) {
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

    SingleInstance<sps::Sleep>::get_instance().sleep(SLEEP_FOREVER);
    return 0;
}
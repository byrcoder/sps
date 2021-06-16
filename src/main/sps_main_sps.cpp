/*****************************************************************************
MIT License
Copyright (c) 2021 byrcoder

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*****************************************************************************/

#include <sps_co.hpp>
#include <sps_core_module.hpp>

#include <sps_install.hpp>
#include <sps_log.hpp>
#include <sps_server_module.hpp>
#include <sps_sync.hpp>
#include <sps_typedef.hpp>
#include <sps_url_protocol.hpp>

sps::PCoreModule core_module;

error_t init_co() {
    return sps::ICoFactory::get_instance().init();
}

error_t init_config(const char* filename) {
    error_t ret           = SUCCESS;
    auto&   protocols     = SingleInstance<sps::UrlProtocol>::get_instance();
    auto    file_reader   = protocols.create("file://" + std::string(filename));

    if (!file_reader) {
        sp_error("url-file-protocol not found");
        return ERROR_URL_PROTOCOL_NOT_EXISTS;
    }

    core_module = std::dynamic_pointer_cast<sps::CoreModule>(
            SingleInstance<sps::ModuleFactoryRegister>::get_instance()
            .create("core", "",  nullptr));

    if ((ret = file_reader->open(std::string(filename))) != SUCCESS) {
        sp_error("Failed open file %s", filename);
        return ret;
    }

    if ((ret = core_module->init_conf(file_reader)) != SUCCESS) {
        sp_error("Failed init config(%s), ret:%d", filename, ret);
        return ret;
    }

    return SUCCESS;
}

error_t install_core() {
    error_t ret = core_module->install();
    return ret;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        sp_error("Argv must greater 1: ./build/sps conf/sps.conf");
        return -1;
    }

    sps::sps_once_install();

    auto ret = init_config(argv[1]);

    if (ret != SUCCESS) {
        return ret;
    }

    if ((ret = init_co()) != SUCCESS) {
        sp_error("Failed co init ret:%d", ret);
        return ret;
    }

    if ((ret = install_core()) != SUCCESS) {
        return ret;
    }

    SingleInstance<sps::Sleep>::get_instance().sleep(SLEEP_FOREVER);
    return 0;
}
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

#include <app/url/sps_url_protocol.hpp>
#include <app/module/sps_module.hpp>

#include <log/sps_log.hpp>

namespace sps {

extern PIModuleFactory sps_modules[];
extern PIURLProtocolFactory sps_url_protocols[];

void init_modules() {
    int i = 0;
    auto &modules = SingleInstance<sps::ModuleFactoryRegister>::get_instance();
    while (sps_modules[i]) {
        modules.reg(std::string(sps_modules[i]->module), sps_modules[i]);
        sp_info("register module %s", sps_modules[i]->module.c_str());
        ++i;
    }

    sp_info("total module %d", i);
}


void init_url_protocol() {
    auto &protocols = SingleInstance<sps::UrlProtocol>::get_instance();

    int i = 0;
    while (sps_url_protocols[i]) {
        protocols.reg(sps_url_protocols[i]);
        sp_info("register url protocol %s", sps_url_protocols[i]->schema);
        ++i;
    }
    sp_info("total url protocol %d", i);
}

// TODO: FIXME thread safe
void sps_once_install() {
    static bool installed = false;
    if (installed) {
        return;
    }

    installed = true;
    init_modules();
    init_url_protocol();
}

}

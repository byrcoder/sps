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

#include <sps_upstream_module.hpp>
#include <memory>

namespace sps {

error_t UpStreamModule::post_conf() {
    auto& obj = SingleInstance<UpStreamModules>::get_instance();
    obj.ups[this->module_name] = std::dynamic_pointer_cast<UpStreamModule>(
            shared_from_this());
    return SUCCESS;
}

std::string UpStreamModule::get_server() {
    return std::static_pointer_cast<UpStreamConfCtx>(conf)->server;
}

PUpStreamModule UpStreamModules::get(std::string& name) {
    auto it = ups.find(name);
    return it == ups.end() ? nullptr : it->second;
}

}  // namespace sps

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

#include <app/core/sps_core_module.hpp>
#include <app/host/sps_host_module.hpp>
#include <app/host/sps_host_loc_module.hpp>

#include <app/http/sps_http_module.hpp>

#include <app/server/sps_server_module.hpp>

#include <app/stream/sps_stream_module.hpp>

#include <app/upstream/sps_upstream_module.hpp>

namespace sps {

#define MODULE_INSTANCE(NAME) (std::make_shared<NAME##ModuleFactory>())

PIModuleFactory sps_modules[] = {
        MODULE_INSTANCE(Core),

        MODULE_INSTANCE(Http),
        MODULE_INSTANCE(Server),
        MODULE_INSTANCE(Host),
        MODULE_INSTANCE(Location),
        MODULE_INSTANCE(Stream),

        MODULE_INSTANCE(UpStream),
        nullptr
};

}
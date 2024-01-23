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

//
// Created by byrcoder on 2021/6/15.
//

#ifndef SPS_HTTP_ADAPTER_PHASE_HANDLER_HPP
#define SPS_HTTP_ADAPTER_PHASE_HANDLER_HPP

#include <memory>

#include <sps_http_api_phase_handler.hpp>
#include <sps_http_proxy_phase_handler.hpp>
#include <sps_http_stream_phase_handler.hpp>

#include <sps_stream_cache.hpp>

namespace sps {

class HttpAdapterPhaseHandler : public IPhaseHandler {
 public:
    HttpAdapterPhaseHandler();

 public:
    error_t handler(IConnection& ctx) override;

 private:
    std::unique_ptr<HttpProxyPhaseHandler>   proxy_handler;
    std::unique_ptr<HttpStreamPhaseHandler>  stream_handler;
    std::unique_ptr<HttpApiPhaseHandler>     api_handler;
};

}  // namespace sps

#endif  // SPS_HTTP_ADAPTER_PHASE_HANDLER_HPP

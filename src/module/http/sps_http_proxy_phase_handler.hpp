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

#ifndef SPS_HTTP_PROXY_PHASE_HANDLER_HPP
#define SPS_HTTP_PROXY_PHASE_HANDLER_HPP

#include <sps_io_url.hpp>
#include <sps_host_phase_handler.hpp>

namespace sps {

// http proxy for upstream
class HttpProxyPhaseHandler : public IPhaseHandler {
 public:
    HttpProxyPhaseHandler();

 public:
    error_t handler(IConnection& ctx) override;
};

}  // namespace sps

#endif  // SPS_HTTP_PROXY_PHASE_HANDLER_HPP

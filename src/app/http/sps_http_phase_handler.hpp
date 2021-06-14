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

#ifndef SPS_HTTP_PHASE_HANDLER_HPP
#define SPS_HTTP_PHASE_HANDLER_HPP

#include <app/url/sps_url.hpp>
#include <app/http/sps_http_parser.hpp>
#include <app/host/sps_host_phase_handler.hpp>
#include <app/server/sps_server_module.hpp>

#include <net/sps_net_socket.hpp>


#define SPS_HTTP_PHASE_CONTINUE 0
#define SPS_HTTP_PHASE_SUCCESS_NO_CONTINUE 1

namespace sps {

// http头部解析
class HttpParsePhaseHandler : public IPhaseHandler {
 public:
    HttpParsePhaseHandler();

 public:
    error_t handler(HostPhaseCtx& ctx) override;
};

// 域名发现
class HttpProxyPhaseHandler : public IPhaseHandler {
 public:
    explicit HttpProxyPhaseHandler();

 public:
    error_t handler(HostPhaseCtx& ctx) override;
};

// 404 响应
class Http404PhaseHandler : public IPhaseHandler, public Single<Http404PhaseHandler> {
 public:
    Http404PhaseHandler();

 public:
    error_t handler(HostPhaseCtx& ctx) override;
};

/**
 * work as nginx
 */
class HttpPhaseHandler : public FifoRegisters<PIPhaseHandler> {
 public:
    error_t handler(HostPhaseCtx& ctx);

 public:
    HttpPhaseHandler();
};

typedef std::shared_ptr<HttpPhaseHandler> PHttpPhaseHandler;

}

#endif  // SPS_HTTP_PHASE_HANDLER_HPP

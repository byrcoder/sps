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

/**
 *
 * kernel IPhaseHandler which work for every connection when accepted
 * every module may has diff IPhaseHandler for example http has HttpParsePhaseHandler,
 * stream submodule has StreamPhaseHandler
 *
 */

#ifndef SPS_HOST_PHASE_HANDLER_HPP
#define SPS_HOST_PHASE_HANDLER_HPP

#include <sps_url.hpp>
#include <sps_host_module.hpp>
#include <sps_io_socket.hpp>
#include <sps_server.hpp>

#define SPS_PHASE_CONTINUE 0
#define SPS_PHASE_SUCCESS_NO_CONTINUE 1

namespace sps {

/**
 * server conn context
 */
struct ConnContext {
    ConnContext(PRequestUrl r, PSocket s, IConnHandler* conn);

    IConnHandler* conn; // rtmp or http conn
    PRequestUrl   req;  // client request. eg: http://github.com/byrcoder
    PSocket       socket; // client socket

    std::string ip;       // client ip
    int         port;     // client port
    PHostModule host;     // router host module
};

/**
 * work as nginx
 */
class IPhaseHandler {
 public:
    explicit IPhaseHandler(const char* name) : name(name) { }
    const char* get_name() { return name; }

 public:
    virtual error_t handler(ConnContext& ctx) = 0;

 private:
    const char* name;
};
typedef std::shared_ptr<IPhaseHandler> PIPhaseHandler;

/**
 * work as nginx http phase
 */
class ServerPhaseHandler : public FifoRegisters<PIPhaseHandler> {
 public:
    error_t handler(ConnContext& ctx);

 public:
    ServerPhaseHandler();
};

typedef std::shared_ptr<ServerPhaseHandler> PServerPhaseHandler;

}

#endif  // SPS_HOST_PHASE_HANDLER_HPP

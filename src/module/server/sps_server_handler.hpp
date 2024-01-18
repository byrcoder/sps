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

#ifndef SPS_SERVER_HANDLER_HPP
#define SPS_SERVER_HANDLER_HPP

#include <memory>

#include <sps_server.hpp>

#define SPS_PHASE_CONTINUE 0
#define SPS_PHASE_SUCCESS_NO_CONTINUE 1

namespace sps {

class IHandlerContext {
 public:
    virtual ~IHandlerContext() = default;

 public:
    PSocket socket;  // client socket
};

/**
 * work as nginx
 */
class IPhaseHandler {
 public:
    explicit IPhaseHandler(const char* name);
    const char* get_name();

 public:
    virtual error_t handler(IHandlerContext& ctx) = 0;

 private:
    const char* name;
};
typedef std::shared_ptr<IPhaseHandler> PIPhaseHandler;

/**
 * work as nginx http phase
 */
class ServerPhaseHandler : public FifoRegisters<PIPhaseHandler> {
 public:
    error_t handler(IHandlerContext& ctx);

 public:
    ServerPhaseHandler();
};

typedef std::shared_ptr<ServerPhaseHandler> PServerPhaseHandler;

}

#endif // SPS_SERVER_HANDLER_HPP

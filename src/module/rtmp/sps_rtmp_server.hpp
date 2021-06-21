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

#ifndef SPS_RTMP_SERVER_HPP
#define SPS_RTMP_SERVER_HPP

#include <sps_server.hpp>
#include <sps_host_phase_handler.hpp>

namespace sps {

class RtmpConnectionHandler : public IConnectionHandler {
 public:
    explicit RtmpConnectionHandler(PSocket io, PServerPhaseHandler& handler);

 public:
    error_t handler() override;

 public:
    PServerPhaseHandler& hd;
};

class RtmpConnectionHandlerFactory : public IConnectionHandlerFactory {
 public:
    explicit RtmpConnectionHandlerFactory(PServerPhaseHandler hd);
    PIConnectionHandler create(PSocket io) override;

 private:
    PServerPhaseHandler handler;
};

}

#endif  // SPS_RTMP_SERVER_HPP
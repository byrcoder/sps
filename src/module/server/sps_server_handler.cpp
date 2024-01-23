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

#include <sps_server_handler.hpp>
#include <sps_log.hpp>

namespace sps {

IConnection::IConnection(PSocket sock) {
    this->socket = std::move(sock);
}

void IConnection::set_context(PIContext ctx) {
    this->context = std::move(ctx);
}

PIContext IConnection::get_context() {
    return context;
}

IPhaseHandler::IPhaseHandler(const char *name) {
    this->name = name;
}

const char *IPhaseHandler::get_name() {
    return name;
}

error_t ServerPhaseHandler::handler(IConnection& ctx) {
    error_t ret = SUCCESS;

    auto& filters = refs();

    for (auto& f : filters) {
        ret = f->handler(ctx);
        if (ret == SPS_PHASE_SUCCESS_NO_CONTINUE) {
            sp_debug("success %s http_server", f->get_name());
            return SUCCESS;
        } else if (ret == SPS_PHASE_CONTINUE) {
            continue;
        } else {
            sp_debug("failed http_server ret:%d", ret);
            return ret;
        }
    }

    return ret;
}

ServerPhaseHandler::ServerPhaseHandler() {
}

PIConnHandler ServerBase::create(PSocket io) {
    return std::make_shared<Connection>(std::move(io), shared_from_this());
}

Connection::Connection(PSocket io, PServerBase server) : IConnHandler(io) {
    this->server = std::move(server);
}

error_t Connection::handler() {
    IConnection ctx(io);
    error_t     ret = SUCCESS;

    do {
        if ((ret = server->handler(ctx)) != SUCCESS) {
            return ret;
        }
        sp_debug("success http_server ret %d", ret);
    } while (true);

    return SUCCESS;
}

}

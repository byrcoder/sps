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

#include <sps_server.hpp>

#include <sps_log.hpp>
#include <st/io/sps_st_io_ssl.hpp>

namespace sps {

IConnHandler::IConnHandler(PSocket io) {
    this->io = std::move(io);
}

void IConnHandler::on_stop() {
    SingleInstance<ConnManager>::get_instance().cancel(shared_from_this());
}

error_t Server::init(PIConnHandlerFactory f,
                     utime_t stm, utime_t rtm) {
    factory         = std::move(f);
    recv_timeout    = rtm;
    send_timeout    = stm;

    return SUCCESS;
}

#ifdef OPENSSL_ENABLED
void Server::init_ssl(const std::string& crt_file, const std::string& key_file,
                      PISSLCertSearch searcher) {
    auto ssl = dynamic_cast<StSSLServerSocket*>(server_socket.get());
    if (ssl) {
        SSLConfig config;
        config.crt_file = crt_file;
        config.key_file = key_file;
        ssl->init_config(config, std::move(searcher));
    }
}
#endif

int Server::listen(std::string ip, int port, bool reuse_port, int backlog,
                   Transport transport) {
    tran        = transport;
    listen_ip   = ip;
    listen_port = port;

    server_socket = SingleInstance<ServerSocketFactory>::get_instance()
            .create_ss(tran);
    return server_socket->listen(ip, port, reuse_port, backlog);
}

error_t Server::accept() {
    do {
        auto io = server_socket->accept();
        auto h  = factory->create(io);

        io->set_recv_timeout(recv_timeout);
        io->set_send_timeout(send_timeout);
        sp_info("success accept new client rcv_timeout: %llu, "
                "send_timeout :%llu", recv_timeout, send_timeout);

        if (ICoFactory::get_instance().start(h) != SUCCESS) {
            sp_error("failed start handler");
            continue;
        }

        SingleInstance<ConnManager>::get_instance().reg(h);
    } while (true);

    return SUCCESS;
}

error_t Server::handler() {
    return accept();
}

}  // namespace sps

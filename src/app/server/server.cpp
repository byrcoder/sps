#include <app/server/server.hpp>
#include <log/logger.hpp>

namespace sps {

SocketManager& SocketManager::get_manage() {
    static SocketManager manager;
    return manager;
}

int SocketManager::add(PISocketHandler h) {
    this->sockets.insert(h);
    return SUCCESS;
}

int SocketManager::remove(PISocketHandler h) {
    sp_info("remove handler h:%p, %lu", h.get(), sockets.size());
    sockets.erase(h);
    return SUCCESS;
}

ISocketHandler::ISocketHandler(PSocket io) {
    this->io = std::move(io);
}

void ISocketHandler::on_stop() {
    SocketManager::get_manage().remove(shared_from_this());
}

Server::Server(PISocketHandlerFactory f, Transport transport) {
    factory         = std::move(f);
    tran            = transport;
}

int Server::listen(std::string ip, int port, bool reuse_port, int backlog) {
    server_socket = ServerSocketFactory::get_instance().create_ss(tran);
    return server_socket->listen(ip, port, reuse_port, backlog);
}

error_t Server::accept() {
    do {
        auto io = server_socket->accept();
        auto h  = factory->create(io);

        sp_info("Success accept new client");

        if (ICoFactory::get_instance().start(h) != SUCCESS) {
            sp_error("Failed start handler");
            continue;
        }

        SocketManager::get_manage().add(h);
    } while(true);

    return SUCCESS;
}

error_t Server::handler() {
    return accept();
}

}

#include <server/server.hpp>
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
    sp_info("remove handler h:%p, %d", h.get(), sockets.size());
    sockets.erase(h);
    return SUCCESS;
}

ISocketHandler::ISocketHandler(PClientSocket io) {
    this->io = std::move(io);
}

void ISocketHandler::on_stop() {
    SocketManager::get_manage().remove(shared_from_this());
}

IServer::IServer(PISocketHandlerFactory factory) {
    this->factory = std::move(factory);
}

error_t IServer::accept() {
    while (true) {
        auto io = do_accept();

        auto h = factory->create(io);

        sp_info("Success accept new client");

        if (ICoFactory::get_instance().start(h) != SUCCESS) {
            sp_error("Failed start handler");
            continue;
        }

        SocketManager::get_manage().add(h);
    }
    return SUCCESS;
}

error_t IServer::handler() {
    return accept();
}

}

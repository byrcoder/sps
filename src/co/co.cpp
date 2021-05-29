#include <co/co.hpp>
#include <co/st/co.hpp>

namespace sps {

static ICoFactory* factory = new STCoFactory();

error_t ICoFactory::start(PICoHandler handler) {
    if (handler == nullptr) { return ERROR_CO_CREATE; }

    auto co = _start(handler);
    if (co == nullptr) { return ERROR_CO_CREATE; }

    handler->co = std::move(co);
    return SUCCESS;
}

ICoFactory& ICoFactory::get_instance() {
    return *factory;
}

}
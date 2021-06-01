#include <co/co.hpp>
#include <st/co/co.hpp>

namespace sps {

error_t ICoFactory::start(PICoHandler handler) {
    if (handler == nullptr) { return ERROR_CO_CREATE; }

    auto co = _start(handler);
    if (co == nullptr) { return ERROR_CO_CREATE; }

    handler->co = std::move(co);
    return SUCCESS;
}

ICoFactory& ICoFactory::get_instance() {
    static STCoFactory f;
    return f;
}

}
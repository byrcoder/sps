#include <st/co/co.hpp>
#include <public.h>

namespace sps {

STCo::STCo(PWICoHandler h) {
    this->h = h;
}

error_t STCo::start() {
    if ((trd = (st_thread_t) st_thread_create(main_co, this, 0, 0)) == nullptr) {
        return ERROR_CO_CREATE;
    }
    return SUCCESS;
}

void* STCo::main_co(void* co) {
    auto p = static_cast<STCo*>(co);
    auto ph = p->h.lock();
    if (!ph) {
        return nullptr;
    }

    ph->on_start();
    error_t err = ph->handler();
    ph->on_stop();

    return p;
}

error_t STCoFactory::init() {
    return st_init();
}

PICo STCoFactory::_start(PICoHandler handler) const {
    auto co = std::make_shared<STCo>(handler);
    if (co->start() != SUCCESS) {
        return nullptr;
    }
    return co;
}

}
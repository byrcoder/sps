#include <co/st/co.hpp>
#include <public.h>

namespace sps {

    STCo::STCo(PICoHandler h) {
        this->h = std::move(h);
    }

    error_t STCo::start() {
        if ((trd = (st_thread_t) st_thread_create(main_co, this, 0, 0)) == nullptr) {
            return ERROR_CO_CREATE;
        }
        return SUCCESS;
    }

    void* STCo::main_co(void* co) {
        auto p = static_cast<STCo*>(co);

        p->h->on_start();
        error_t err = p->h->handler();
        p->h->on_stop();

        return p;
    }

    PICo STCoFactory::start(PICoHandler handler) const {
        return std::make_shared<STCo>(handler);
    }
}
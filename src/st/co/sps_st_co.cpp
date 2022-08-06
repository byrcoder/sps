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

#include <sps_st_co.hpp>

#include <public.h>

#include <memory>

namespace sps {

int STContext::get_id() {
    auto self = st_thread_self();
    if (!self) {
        return -1;
    }

    auto it   = ids.find(self);
    return it != ids.end() ? it->second : -1;
}

void STContext::set_id() {
    static int id;
    ids[st_thread_self()] = id++;
}

void STContext::remove_id() {
    ids.erase(st_thread_self());
}

STCo::STCo(PWICoHandler h) {
    this->h = h;
}

error_t STCo::start() {
    if ((trd = (st_thread_t) st_thread_create(main_co, this,
            0, 0)) == nullptr) {
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

#ifndef ST_THREAD_HAS_CONTEXT_ID
    STContext::get_instance().set_id();
#endif

    ph->on_start();
    error_t err = ph->handler();
    ph->on_stop();

#ifndef ST_THREAD_HAS_CONTEXT_ID
    STContext::get_instance().remove_id();
#endif
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

int get_context_id() {
#ifdef ST_THREAD_HAS_CONTEXT_ID
    return st_context();
#else
    return STContext::get_instance().get_id();
#endif
}

}  // namespace sps

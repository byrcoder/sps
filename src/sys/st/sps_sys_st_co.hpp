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

#ifndef SPS_SYS_ST_CO_HPP
#define SPS_SYS_ST_CO_HPP

#include "sys/sps_sys_co.hpp"

#include "public.h"

#include <unordered_map>

namespace sps {

class STContext : public SingleInstance<STContext> {
 private:
    std::unordered_map<st_thread_t, int> ids;

 public:
    int  get_id();
    void set_id();
    void remove_id();
};

class STCo : public ICo {
 public:
    explicit STCo(PWICoHandler h);
    error_t start() override;

 public:
    static void* main_co(void* st_co);

 private:
    PWICoHandler h;
    st_thread_t trd;
};

class STCoFactory : public ICoFactory {
 public:
    error_t init() override;

    PICo _start(PICoHandler handler) const override;
};

}  // namespace sps

#endif  // SPS_SYS_ST_CO_HPP

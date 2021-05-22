#ifndef SPS_CO_ST_CO_HPP
#define SPS_CO_ST_CO_HPP

#include "co/co.hpp"

extern "C" {
#include <public.h>
};

namespace sps {

class STCo : public ICo {
 public:
    STCo(PICoHandler h);

    error_t start() override;

    static void* main_co(void* st_co);

 private:
    PICoHandler h;
    st_thread_t trd;
};

class STCoFactory : public ICoFactory {
 public:
    PICo start(PICoHandler handler) const override;
};

}

#endif // SPS_CO_ST_CO_HPP

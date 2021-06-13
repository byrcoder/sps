#ifndef SPS_ST_CO_HPP
#define SPS_ST_CO_HPP

#include <co/sps_co.hpp>

extern "C" {
#include <public.h>
};

namespace sps {

class STCo : public ICo {
 public:
    STCo(PWICoHandler h);
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

}

#endif  // SPS_ST_CO_HPP

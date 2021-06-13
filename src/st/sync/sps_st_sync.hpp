#ifndef SPS_ST_SYNC_HPP
#define SPS_ST_SYNC_HPP

#include <sync/sps_sync.hpp>

namespace sps {

class StCondition : public ICondition {
 public:
    StCondition()             {         cond = st_cond_new();   }
    ~StCondition() override   {         st_cond_destroy(cond);  }

 private:
    int wait(utime_t timeout) override {   return st_cond_timedwait(cond, timeout);   }
    int signal() override              {   return st_cond_broadcast(cond);            }

 private:
    st_cond_t cond;
};

class StConditionFactory : public IConditionFactory {
 public:
    PCondition create_condition() override   { return std::make_shared<StCondition>(); }
};

}

#endif  // SPS_ST_SYNC_HPP

#include <sync/sps_sync.hpp>
#include <st/sync/sps_st_sync.hpp>

namespace sps {

int Sleep::sleep(int seconds) {
    return st_sleep(seconds);
}

int Sleep::usleep(utime_t tm) {
    return st_usleep(tm);
}

IConditionFactory & IConditionFactory::get_instance() {
    static StConditionFactory f;
    return f;
}

}
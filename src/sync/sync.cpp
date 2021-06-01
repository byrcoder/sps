#include <sync/sync.hpp>
#include <st/sync/sync.hpp>

namespace sps {

IConditionFactory & IConditionFactory::get_instance() {
    static StConditionFactory f;
    return f;
}

}
#include <co/co.hpp>
#include <co/st/co.hpp>

namespace sps {

static ICoFactory* factory = new STCoFactory();

const ICoFactory& ICoFactory::get_instance() {
    return *factory;
}

}
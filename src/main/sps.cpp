#include <co/co.hpp>
#include <public.h>

int main(int argc, char* argv[]) {
    st_init();
    sps::ICoFactory::get_instance().start(nullptr);
    st_sleep(-1);
    return 0;
}
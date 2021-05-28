#ifndef SPS_FLV_HPP
#define SPS_FLV_HPP

#include <net/io.hpp>
#include <protocol//msg.hpp>

namespace sps {

class FLV {
 public:
    FLV(PIReader rd);

 public:
    error_t read_header(char buf[9]);
    error_t read_tag(char* buf, int len, int& nread);

 private:
    PIReader rd;
};

}

#endif // SPS_FLV_HPP

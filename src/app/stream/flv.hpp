#ifndef SPS_FLV_HPP
#define SPS_FLV_HPP

#include <app/stream/msg.hpp>
#include <app/stream/protocol.hpp>
#include <net/io.hpp>
#include <vector>

namespace sps {

class Flv {
 public:
    Flv(PIReader rd);

 public:
    error_t read_header(char buf[9]);
    error_t read_tag(char* buf, int len, int& nread);

 private:
    PIReader rd;
};

typedef std::shared_ptr<Flv> PFlv;

class FlvProtocol : public IProtocol {
 public:
    FlvProtocol(PIReader rd);

 public:
    error_t read_header(PIBuffer& buffer)  override;
    error_t read_message(PIBuffer& buffer) override;
    error_t read_tail(PIBuffer& buffer)    override;

 private:
    PFlv flv;
    std::vector<char> buf;

 public:
    static const int max_len = 1 * 1024 * 1024;
};

}

#endif // SPS_FLV_HPP

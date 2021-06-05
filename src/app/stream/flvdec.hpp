#ifndef SPS_FLVDEC_HPP
#define SPS_FLVDEC_HPP

#include <app/stream/msg.hpp>
#include <app/stream/dec.hpp>
#include <net/net_io.hpp>
#include <vector>

namespace sps {

class FlvDecoder : public IAvDemuxer {
 public:
    FlvDecoder(PIReader rd);

 public:
    error_t read_header(PIBuffer& buffer)  override;
    error_t read_message(PIBuffer& buffer) override;
    error_t read_tail(PIBuffer& buffer)    override;
    error_t probe(PIBuffer& buffer)        override;

 private:
    std::vector<char> buf;
    PIReader rd;

 public:
    static const int max_len = 1 * 1024 * 1024;
};

class FlvInputFormat : public IAVInputFormat {
 public:
    FlvInputFormat();

 public:
    PIAvDemuxer create(PIURLProtocol s) override;

};

}

#endif // SPS_FLVDEC_HPP

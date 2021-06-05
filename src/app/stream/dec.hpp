#ifndef SPS_PROTOCOL_PROTOCOL_HPP
#define SPS_PROTOCOL_PROTOCOL_HPP

#include <app/url/url.hpp>
#include <cache/buffer.hpp>

#include <net/io.hpp>

#include <typedef.hpp>
#include <list>
#include <app/url/protocol.hpp>

#define PROTOCOL_EOF 2000

namespace sps {

class IAVInputFormat;

class IAvDemuxer;
typedef std::shared_ptr<IAvDemuxer> PIAvDemuxer;

// 解封装协议
class IAvDemuxer {
 public:
    virtual ~IAvDemuxer() = default;

 public:
    const IAVInputFormat* name();

 public:
    virtual error_t read_header(PIBuffer& buffer)  = 0;
    virtual error_t read_message(PIBuffer& buffer) = 0;
    virtual error_t read_tail(PIBuffer& buffer)    = 0;
    virtual error_t probe(PIBuffer& buffer)        = 0;

 public:
    const IAVInputFormat* fmt;
};

// IAvDemuxer创建方法，是否匹配的方法
class IAVInputFormat {
 public:
    IAVInputFormat(const char*name, const char* ext);
    virtual ~IAVInputFormat() = default;

 public:
    virtual bool match(const char* ext) const;

 public:
    virtual PIAvDemuxer create(PIURLProtocol p) = 0;

 public:
    const char* name;
    const char* ext_name;
};

typedef std::shared_ptr<IAVInputFormat> PIAVInputFormat;

// c++ 没有无反射，每个IAvDeumxer的创建需要实现工厂方法
class AvDemuxerFactory : public Registers<AvDemuxerFactory, PIAVInputFormat> {
 public:
    AvDemuxerFactory();

 public:
    error_t init();

 public:
    // TODO: to impl probe media format
    PIAvDemuxer probe(PIURLProtocol p, PRequestUrl url);

 public:
    PIAvDemuxer create(PIURLProtocol p, PRequestUrl url);
    PIAvDemuxer create(PIURLProtocol p, const std::string& url);
};

}

#endif  // SPS_PROTOCOL_PROTOCOL_HPP

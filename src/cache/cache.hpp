#ifndef SPS_CACHE_HPP
#define SPS_CACHE_HPP

#include <list>
#include <map>
#include <memory>
#include <string>

#include <cache/buffer.hpp>
#include <typedef.hpp>

namespace sps {

class ICacheStream {
 public:
    virtual ~ICacheStream() = default;

 public:
    virtual error_t put(PIBuffer pb) = 0;
    virtual int dump(std::list<PIBuffer>& vpb) = 0;
    virtual int size() = 0;
};

typedef std::shared_ptr<ICacheStream> PICacheStream;

class CacheStream : public ICacheStream {
 public:
    error_t put(PIBuffer pb) override;
    int dump(std::list<PIBuffer>& vpb) override;
    int size() override ;

 private:
    std::list<PIBuffer> pbs;
};

class ICache {
 public:
    virtual ~ICache() = default;

 public:
    virtual PICacheStream get(const std::string& name) = 0;
    virtual error_t       put(const std::string& name, PICacheStream cs) = 0;
};

class InfiniteCache : public ICache {
 public:
    PICacheStream get(const std::string& name) override;
    error_t       put(const std::string& name, PICacheStream cs) override;

 private:
    std::map<std::string, PICacheStream> css;
};

}

#endif  // SPS_CACHE_HPP

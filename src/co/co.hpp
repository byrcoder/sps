#ifndef SPS_CO_CO_HPP
#define SPS_CO_CO_HPP

#include <string>
#include <memory>

#include "typedef.hpp"

namespace sps {

class ICoHandler {
 public:
    virtual ~ICoHandler() = default;
    virtual error_t handler() = 0;

 public:
    virtual void on_start() { };
    virtual void on_stop()  { };
};

class ICo {
 public:
    ICo() = default;
    virtual ~ICo() = default;

 public:
    virtual error_t start() = 0;
};

typedef std::shared_ptr<ICo> PICo;
typedef std::shared_ptr<ICoHandler> PICoHandler;

class ICoFactory {
 public:
    virtual PICo start(PICoHandler handler) const = 0;

 public:
    static const ICoFactory& get_instance();
};

};

#endif  // SPS_CO_CO_HPP

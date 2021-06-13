#ifndef SPS_CO_HPP
#define SPS_CO_HPP

#include <string>
#include <memory>

#include "typedef.hpp"

namespace sps {

class ICo {
 public:
    ICo() = default;
    virtual ~ICo() = default;

 public:
    virtual error_t start() = 0;
};
typedef std::shared_ptr<ICo> PICo;


class ICoHandler {
 public:
    friend class ICoFactory;

 public:
    virtual ~ICoHandler() = default;
    virtual error_t handler() = 0;

 public:
    virtual void on_start() { };
    virtual void on_stop()  { };

 private:
    PICo co;
};
typedef std::shared_ptr<ICoHandler> PICoHandler;
typedef std::weak_ptr<ICoHandler>   PWICoHandler;

class ICoFactory {
 public:
    ICoFactory() = default;

 public:
    virtual error_t init() = 0;

 public:
    virtual PICo _start(PICoHandler handler) const = 0;
    error_t start(PICoHandler handler);

 public:
    static ICoFactory& get_instance();
};

};

#endif  // SPS_CO_HPP

/*****************************************************************************
MIT License
Copyright (c) 2021 byrcoder

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*****************************************************************************/

#ifndef SPS_CO_HPP
#define SPS_CO_HPP

#include <string>
#include <memory>

#include <sps_typedef.hpp>

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
    virtual void on_start() { }
    virtual void on_stop()  { }

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

};  // namespace sps

#endif  // SPS_CO_HPP

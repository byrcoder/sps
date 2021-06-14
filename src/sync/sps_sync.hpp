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

#ifndef SPS_SYNC_HPP
#define SPS_SYNC_HPP

#include <memory>
#include <set>

#include <public.h> // st
#include <sps_typedef.hpp>
#include <log/sps_log.hpp>

#define SLEEP_FOREVER -1

namespace sps {

class Sleep : public SingleInstance<Sleep> {
 public:
    int sleep(int seconds);
    int usleep(utime_t tm);
};

class ICondition {
 public:
    virtual ~ICondition() = default;

 public:
    virtual int wait(utime_t timeout) = 0;
    virtual int signal() = 0;
};

typedef std::shared_ptr<ICondition> PCondition;

class IConditionFactory {
 public:
    static IConditionFactory& get_instance();

 public:
    virtual PCondition create_condition() = 0;

 public:
    IConditionFactory() = default;
    virtual ~IConditionFactory() = default;
};

template <class T>
class Subscriber {
 public:
    Subscriber()     {  cond = IConditionFactory::get_instance().create_condition(); }

 public:
    int wait(utime_t timeout)      {  return cond->wait(timeout); }
    int on_event(T& obj)           {  do_event(obj); cond->signal();  return SUCCESS; }
    virtual int do_event(T& obj)   {  return SUCCESS;        }

 protected:
    PCondition cond;
};

template <class T>
struct WeakPtrLessThan
{
    bool operator() ( const std::weak_ptr<T>& lhs, const std::weak_ptr<T>& rhs) const {
        auto lptr = lhs.lock(), rptr = rhs.lock();
        if (!rptr) return false;  // nothing after expired pointer
        if (!lptr) return true;   // every not expired after expired pointer
        return lptr.get() < rptr.get();
    }
};

template <class T>
class Publisher {
 public:
    void publish(T& o) {
        sp_info("publish ... subs:%lu", subs.size());

        for (auto it = subs.begin(); it != subs.end(); ) {
            auto s = (*it).lock();

            if (!s) {
                subs.erase(it++);
            } else {
                s->on_event(o);
                ++it;
            }
        }
    }

    void sub(std::weak_ptr<Subscriber<T> > s) {
        subs.insert(s);
    }

    void cancel(std::weak_ptr<Subscriber<T> > s) {
        subs.erase(s);
    }

 private:
    std::set<std::weak_ptr<Subscriber<T>>, WeakPtrLessThan<Subscriber<T>> > subs;
};

}


#endif  // SPS_SYNC_HPP

#ifndef SPS_SYNC_SYNC_HPP
#define SPS_SYNC_SYNC_HPP

#include <memory>
#include <set>

#include <public.h> // st
#include <typedef.hpp>
#include <log/logger.hpp>

namespace sps {

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


#endif  // SPS_SYNC_SYNC_HPP

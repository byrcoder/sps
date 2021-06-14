#ifndef SPS_SPS_TYPEDEF_HPP
#define SPS_SPS_TYPEDEF_HPP

#include <list>
#include <map>
#include <set>

#include <sps_error.hpp>

#ifndef __unused

#define __unused __attribute__((unused))

#endif

typedef int error_t;
typedef unsigned long long utime_t;

#define SUCCESS 0L

#define ERROR_THREAD_DISPOSED              20
#define ERROR_THREAD_STARTED               21
#define ERROR_ST_CREATE_CYCLE_THREAD       22
#define ERROR_THREAD_TERMINATED            23
#define ERROR_THREAD_INTERRUPED            24

#define ERROR_CO_CREATE 100









template<class T>
class SingleInstance {
 public:
    static T& get_instance() {
        static T obj;
        return obj;
    }

    static std::shared_ptr<T> get_instance_share_ptr() {
        static std::shared_ptr<T> ptr = std::shared_ptr<T>(&get_instance());
        return ptr;
    }
};

template<class T>
class Single {
 public:
    friend class SingleInstance<T>;
 protected:
    virtual ~Single() = default;
    Single() = default;
 private:
    Single(Single& ) = default;
};

template<class T, class S>
class Registers : public Single<T> {
 public:
    void reg(const S& obj) {
        objs.insert(obj);
    }

    void cancel(const S& obj) {
        objs.erase(obj);
    }

    std::set<S>& refs() {
        return objs;
    }
 protected:
    std::set<S> objs;
};

template<class Key, class Object>
class KeyRegisters : public Single<Key> {
 public:
    void reg(const Key& key, const Object& obj) {
        objs[key] = obj;
    }

    void cancel(const Key& key) {
        objs.erase(key);
    }

    Object* get(const Key& key) {
        auto it = objs.find(key);
        return it == objs.end() ? nullptr : &(it->second);
    }

 private:
    std::map<Key, Object> objs;
};

template<class S>
class FifoRegisters  {
 public:
    void reg(const S& obj) {
        objs.push_back(obj);
    }

    void cancel(const S& obj) {
        auto it = std::find(objs.begin(), objs.end(), obj);
        if (it != objs.end()) {
            objs.erase(it);
        }
    }

    std::list<S>& refs() {
        return objs;
    }
 protected:
    std::list<S> objs;
};

#endif // SPS_SPS_TYPEDEF_HPP

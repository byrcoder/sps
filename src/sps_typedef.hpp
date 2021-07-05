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

#ifndef SPS_SPS_TYPEDEF_HPP
#define SPS_SPS_TYPEDEF_HPP

#include <list>
#include <map>
#include <memory>
#include <set>

#include <sps_error.hpp>

#ifndef __unused
#define __unused __attribute__((unused))
#endif

typedef int error_t;
typedef uint64_t utime_t;

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
    Single(Single&) = default;
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

#endif  // SPS_SPS_TYPEDEF_HPP

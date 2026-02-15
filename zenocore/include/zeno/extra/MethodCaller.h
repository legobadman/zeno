#pragma once

#include <string>
#include <memory>
#include <zeno/core/Graph.h>
#include <iobject2.h>
#include <zeno/core/ZNode.h>
#include <zeno/funcs/LiterialConverter.h>
#include <zeno/types/FunctionObject.h>
#include <zeno/utils/safe_dynamic_cast.h>
#include <zeno/utils/safe_at.h>

namespace zeno {
#if 0
struct MethodCaller {
    std::function<std::map<std::string, zany2>(std::map<std::string, zany2> const &)> func;
    std::map<std::string, zany2> params;
    bool called = false;

    MethodCaller(std::shared_ptr<DictObject> const &callbacks, std::string const &method)
        : func(safe_dynamic_cast<FunctionObject>(safe_at(callbacks->lut, method, "callbacks"), "callback")->func)
    {}

    MethodCaller(std::shared_ptr<DictObject> const &callbacks, std::string const &method, std::map<std::string, zany2> const &defl)
        : func(!callbacks->lut.count(method) ?
               [defl] (std::map<std::string, zany2> const &) -> std::map<std::string, zany2> { return defl; } :
               safe_dynamic_cast<FunctionObject>(callbacks->lut.at(method), "callback")->func)
    {}

    MethodCaller &set(std::string const &id, std::shared_ptr<IObject2> obj) {
        params[id] = std::move(obj);
        return *this;
    }

    template <class T>
    MethodCaller &set2(std::string const &id, T const &val) {
        params[id] = objectFromLiterial(val);
        return *this;
    }

    std::shared_ptr<IObject2> get(std::string const &key) {
        call();
        return safe_at(params, key, "return key `" + key + "` of method");
    }

    bool has(std::string const &key) {
        call();
        return params.find(key) != params.end();
    }

    template <class T>
    std::shared_ptr<T> get(std::string const &key) {
        return safe_dynamic_cast<T>(get(key), "return key `" + key + "` of method");
    }

    template <class T>
    T get2(std::string const &key) {
        return objectToLiterial<T>(get(key), "return key `" + key + "` of method");
    }

    template <class T>
    T get2(std::string const &key, T defl) {
        return has(key) ? get2<T>(key) : defl;
    }

    MethodCaller &call() {
        if (!called) {
            params = func(params);
            called = true;
        }
        return *this;
    }
};
#endif
}

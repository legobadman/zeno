#pragma once

#include <zeno/utils/vec.h>
#include <zeno/core/IObject.h>
#include <zeno/utils/Exception.h>
#include <variant>
#include <zeno/core/data.h>

namespace zeno {

struct NumericObject : IObjectClone<NumericObject> {
    NumericValue value;

    NumericObject() = default;
    NumericObject(NumericValue const &value) : value(value) {}

    void Delete() { delete this; }

    NumericValue &get() {
        return value;
    }

    NumericValue const &get() const {
        return value;
    }

    template <class T>
    T get() const {
      return std::visit([] (auto const &val) -> T {
          using V = std::decay_t<decltype(val)>;
          if constexpr (!std::is_constructible_v<T, V>) {
              throw makeError<TypeError>(typeid(T), typeid(V), "NumericObject::get<T>");
          } else {
              return T(val);
          }
      }, value);
    }

    template <class T>
    bool is() const {
      return std::holds_alternative<T>(value);
    }

    template <class T>
    void set(T const &x) {
      value = x;
    }
};


}

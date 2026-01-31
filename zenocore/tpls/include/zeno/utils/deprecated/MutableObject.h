#pragma once

#include <iobject2.h>
#include <zeno/utils/Any.h>
#include <string>

namespace zeno {

struct MutableObject : IObjectClone<MutableObject> {
  zany2 value;

  template <class T>
  T get() const {
    return safe_any_cast<T>(value, "MutableObject::get ");
  }

  template <class T>
  void set(T const &x) {
    value = x;
  }

  template <class T>
  void set(T &&x) {
    value = std::move(x);
  }
};

}

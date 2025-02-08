#pragma once

#include <zeno/core/IObject.h>
#include <zeno/funcs/LiterialConverter.h>
#include <memory>
#include <string>
#include <map>

namespace zeno {

struct DictObject : IObjectClone<DictObject> {

    DictObject() = default;

  std::map<std::string, zany> lut;

  //一次计算中发生变化的元素记录，如果内部元素是dict/list，不记录嵌套的情况，只标记modify，详细信息通过访问子元素对象的这三个便可知。
  std::set<std::string> m_modify, m_new_added, m_new_removed; 


  DictObject(const DictObject& dictObj) {
      m_key = dictObj.m_key;
      for (auto& [str, obj] : dictObj.lut) {
          auto cloneobj = obj->clone();
          cloneobj->update_key(obj->key());
          lut.insert({str, cloneobj});
      }
      m_modify = dictObj.m_modify;
      m_new_added = dictObj.m_new_added;
      m_new_removed = dictObj.m_new_removed;
  }

  std::shared_ptr<IObject> clone_by_key(std::string const& prefix) override {
      //不修改自身
      std::shared_ptr<DictObject> spDict = std::make_shared<DictObject>();
      spDict->update_key(prefix + '\\' + this->m_key);
      for (const auto& uuid : m_modify) {
          spDict->m_modify.insert(prefix + '\\' + uuid);
      }
      for (const auto& uuid : m_new_added) {
          spDict->m_new_added.insert(prefix + '\\' + uuid);
      }
      for (const auto& uuid : m_new_removed) {
          spDict->m_new_removed.insert(prefix + '\\' + uuid);
      }
      for (auto& [key, spObject] : lut) {
          std::string new_key = prefix + '\\' + key;
          auto newObj = spObject->clone_by_key(prefix);
          spDict->lut.insert(std::make_pair(key, newObj));
      }
      return spDict;
  }

  template <class T = IObject>
  std::map<std::string, std::shared_ptr<T>> get() const {
      std::map<std::string, std::shared_ptr<T>> res;
      for (auto const &[key, val]: lut) {
          res.emplace(key, safe_dynamic_cast<T>(val));
      }
      return res;
  }

  template <class T>
  std::map<std::string, T> getLiterial() const {
      std::map<std::string, T> res;
      for (auto const &[key, val]: lut) {
          res.emplace(key, objectToLiterial<T>(val));
      }
      return res;
  }
};

}

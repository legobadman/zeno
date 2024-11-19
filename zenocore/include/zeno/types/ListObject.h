#pragma once

#include <zeno/core/IObject.h>
#include <zeno/funcs/LiterialConverter.h>
#include <vector>
#include <memory>
#include <set>


namespace zeno {

struct ListObject : IObjectClone<ListObject> {

  ListObject() = default;

  explicit ListObject(std::vector<zany> arrin) : m_objects(std::move(arrin)) {
  }

  std::shared_ptr<IObject> clone_by_key(std::string const& prefix) override {
      std::shared_ptr<ListObject> spList = std::make_shared<ListObject>();
      spList->update_key(prefix + '\\' + m_key);
      for (const auto& uuid : m_modify) {
          spList->m_modify.insert(prefix + '\\' + uuid);
      }
      for (const auto& uuid : m_new_added) {
          spList->m_new_added.insert(prefix + '\\' + uuid);
      }
      for (const auto& uuid : m_new_removed) {
          spList->m_new_removed.insert(prefix + '\\' + uuid);
      }

      for (auto& spObject : m_objects) {
          auto newObj = spObject->clone_by_key(prefix);
          spList->m_objects.push_back(newObj);
      }
      return spList;
  }

  template <class T = IObject>
  std::vector<std::shared_ptr<T>> get() const {
      std::vector<std::shared_ptr<T>> res;
      for (auto const &val: m_objects) {
          res.push_back(safe_dynamic_cast<T>(val));
      }
      return res;
  }

  template <class T = IObject>
  std::vector<T *> getRaw() const {
      std::vector<T *> res;
      for (auto const &val: m_objects) {
          res.push_back(safe_dynamic_cast<T>(val.get()));
      }
      return res;
  }

  void resize(const size_t sz) {
      m_objects.resize(sz);
  }

  void append(zany spObj) {
      m_objects.push_back(spObj);
      spObj->set_parent(this);
      //m_ptr2Index.insert(std::make_pair((uint16_t)spObj.get(), m_objects.size()));
  }

  void append(zany&& spObj) {
      m_objects.push_back(spObj);
      spObj->set_parent(this);
      //m_ptr2Index.insert(std::make_pair((uint16_t)spObj.get(), m_objects.size()));
  }

  zany get(int index) const {
      if (0 > index || index >= m_objects.size())
          return nullptr;
      return m_objects[index];
  }

  void set(const std::vector<zany>& arr) {
      m_objects = arr;
  }

  void set(size_t index, zany&& obj) {
      if (0 > index || index >= m_objects.size())
          return;
      m_objects[index] = obj;
  }

  void mark_dirty(int index) {
      dirtyIndice.insert(index);
  }

  bool has_dirty(int index) const {
      return dirtyIndice.count(index);
  }

  size_t size() const {
      return m_objects.size();
  }

  size_t dirtyIndiceSize() {
      return dirtyIndice.size();
  }

  void emplace_back(zany&& obj) {
      append(obj);
  }

  void push_back(zany&& obj) {
      append(obj);
  }

  void push_back(const zany& obj) {
      append(obj);
  }

  void clear() {
      m_objects.clear();
  }

  template <class T>
  std::vector<T> get2() const {
      std::vector<T> res;
      for (auto const &val: m_objects) {
          res.push_back(objectToLiterial<T>(val));
      }
      return res;
  }

  template <class T>
  [[deprecated("use get2<T>() instead")]]
  std::vector<T> getLiterial() const {
      return get2<T>();
  }
    std::set<std::string> m_modify, m_new_added, m_new_removed; //一次计算中发生变化的元素记录。

private:
    std::vector<zany> m_objects;
    std::set<int> dirtyIndice;                        //该list下dirty的obj的index
    //std::map<std::string, int> nodeNameArrItemMap;    //obj所在的节点名到obj在m_objects中索引的map
    //std::map<uint16_t, int> m_ptr2Index;
};

}

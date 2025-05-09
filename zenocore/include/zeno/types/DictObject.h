#pragma once

#include <memory>
#include <string>
#include <map>
#include <set>

#include <zeno/core/IObject.h>
#include <zeno/funcs/LiterialConverter.h>

namespace zeno {

struct ZENO_API DictObject : IObjectClone<DictObject> {
    DictObject();
    DictObject(const DictObject& dictObj);

    ~DictObject();
    void Delete() override;

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

    //TODO: 目前还没有自开发的map和set，所以这里没法保证abi兼容
    std::map<std::string, zany> lut;     //TMD全暴露出去了，想改都不好改
    //一次计算中发生变化的元素记录，如果内部元素是dict/list，不记录嵌套的情况，只标记modify，详细信息通过访问子元素对象的这三个便可知。
    std::set<std::string> m_modify, m_new_added, m_new_removed;

private:
    void clear_children();
};

ZENO_API zeno::SharedPtr<DictObject> create_DictObject();

}

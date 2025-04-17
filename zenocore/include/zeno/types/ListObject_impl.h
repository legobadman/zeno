#pragma once

#include <zeno/core/coredata.h>
#include <zeno/types/ListObject.h>
#include <zeno/utils/safe_dynamic_cast.h>
#include <set>
#include <vector>
#include <string>

namespace zeno {

    struct ZENO_API ListObject_impl {

        ListObject_impl() = default;
        ListObject_impl(const ListObject_impl& listobj);
        explicit ListObject_impl(std::vector<zany> arrin) : m_objects(std::move(arrin)) {}

        void remove_children();
        std::vector<std::string> paths() const;
        void resize(const size_t sz);
        void append(zany spObj);
        void append(zany&& spObj);
        zany get(int index) const;
        void set(const std::vector<zany>& arr);
        void set(size_t index, zany obj);
        void mark_dirty(int index);
        bool has_dirty(int index) const;
        size_t size() const;
        size_t dirtyIndiceSize();
        void emplace_back(zany&& obj);
        void push_back(zany&& obj);
        void push_back(const zany& obj);
        void clear();

        template <class T = IObject>
        std::vector<zeno::SharedPtr<T>> get() const {
            std::vector<zeno::SharedPtr<T>> res;
            for (auto const& val : m_objects) {
                res.push_back(safe_dynamic_cast<T>(val));
            }
            return res;
        }

        template <class T = IObject>
        std::vector<T*> getRaw() const {
            std::vector<T*> res;
            for (auto const& val : m_objects) {
                res.push_back(safe_dynamic_cast<T>(val.get()));
            }
            return res;
        }

        template <class T>
        std::vector<T> get2() const {
            std::vector<T> res;
            for (auto const& val : m_objects) {
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
        std::vector<zany> m_objects;
        std::set<int> dirtyIndice;                        //该list下dirty的obj的index
        //std::map<std::string, int> nodeNameArrItemMap;    //obj所在的节点名到obj在m_objects中索引的map
        //std::map<uint16_t, int> m_ptr2Index;
    };
}
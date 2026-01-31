#pragma once

#include <zeno/core/coredata.h>
#include <zeno/types/ListObject.h>
#include <zeno/utils/safe_dynamic_cast.h>
#include <zeno/funcs/LiterialConverter.h>
#include <set>
#include <vector>
#include <string>

namespace zeno {

    struct ZENO_API ListObject_impl {

        ListObject_impl() = default;
        ListObject_impl(const ListObject_impl& listobj);
        ListObject_impl(const std::vector<zany2>& arrin);

        void remove_children();
        std::vector<std::string> paths() const;
        void resize(const size_t sz);
        void append(zany2&& spObj);
        IObject2* get(int index) const;
        zany2 move(int index);
        void set(const std::vector<zany2>& arr);
        void set(size_t index, zany2&& obj);
        void mark_dirty(int index);
        bool has_dirty(int index) const;
        size_t size() const;
        size_t dirtyIndiceSize();
        void emplace_back(zany2&& obj);
        void push_back(zany2&& obj);
        void clear();

        template <class T = IObject2>
        std::vector<T*> get() const {
            std::vector<T*> res;
            for (auto const& val : m_objects) {
                res.push_back(dynamic_cast<T*>(val.get()));
            }
            return res;
        }

        template <class T = IObject2>
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

        ListObject_impl& operator=(const ListObject_impl&) = delete;

        std::set<std::string> m_modify, m_new_added, m_new_removed; //一次计算中发生变化的元素记录。
        std::vector<zany2> m_objects;
        std::set<int> dirtyIndice;                        //该list下dirty的obj的index
        //std::map<std::string, int> nodeNameArrItemMap;    //obj所在的节点名到obj在m_objects中索引的map
        //std::map<uint16_t, int> m_ptr2Index;
    };
}
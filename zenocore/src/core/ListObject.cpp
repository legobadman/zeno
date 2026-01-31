#include <zeno/utils/helper.h>
#include <zeno/types/ListObject.h>
#include <zeno/types/ListObject_impl.h>
#include <zeno/funcs/LiterialConverter.h>
#include <zeno/utils/interfaceutil.h>
#include <memory>


namespace zeno
{
    ListObject::ListObject() {}

    ListObject::ListObject(const ListObject& rhs) {

        dirtyIndice = rhs.dirtyIndice;

        char buf[1024];

        m_objects.resize(rhs.size());
        for (int i = 0; i < rhs.size(); ++i) {
            IObject2* rhsObj = rhs.get(i);
            m_objects[i] = zany2(rhsObj->clone());  //raii
            size_t sz = rhsObj->key(buf, sizeof(buf));
            m_objects[i]->update_key(buf);
        }

        m_modify = rhs.m_modify;
        m_new_added = rhs.m_new_added;
        m_new_removed = rhs.m_new_removed;

        rhs.key(buf, sizeof(buf));
        update_key(buf);
    }

    ListObject::~ListObject() {
        clear();
    }

    void ListObject::clear() {
        m_new_added.clear();
        m_modify.clear();
        m_new_removed.clear();
        m_objects.clear();
    }

    bool ListObject::has_change_info() const {
        return !m_modify.empty() || !m_new_added.empty() || !m_new_removed.empty();
    }

    bool ListObject::empty() const {
        return m_objects.empty();
    }

    void ListObject::update_key(const char* key) {
        if (!key || strlen(key) == 0) return;
        for (int i = 0; i < m_objects.size(); i++) {
            auto& obj = m_objects[i];
            if (get_object_key(obj.get()).empty()) {
                std::string newkey = m_key + "/" + std::to_string(i);
                obj->update_key(m_key.c_str());
            }
        }
    }

    size_t ListObject::get_items(IObject2** buf, size_t cap) const {
        return 0;
    }

    size_t ListObject::get_int_arr(int* buf, size_t cap) const {
        return 0;
    }

    size_t ListObject::get_float_arr(float* buf, size_t cap) const {
        return 0;
    }

    size_t ListObject::get_string_arr(char** buf, size_t cap) const {
        return 0;
    }

    size_t ListObject::size() const {
        return m_objects.size();
    }

    void ListObject::resize(size_t sz) {
        m_objects.resize(sz);
    }

    IObject2* ListObject::get(size_t index) const {
        if (0 > index || index >= m_objects.size())
            return nullptr;
        return m_objects[index].get();
    }

    std::vector<IObject2*> ListObject::get() {
        std::vector<IObject2*> vec;
        vec.reserve(m_objects.size());
        for (const auto& obj : m_objects) {
            vec.push_back(obj.get());
        }
        return vec;
    }

    void ListObject::push_back(IObject2* detached_obj) {
        m_objects.push_back(zany2(detached_obj));
    }

    void ListObject::push_back2(zany2&& detached_obj) {
        m_objects.push_back(std::move(detached_obj));
    }

    //void ListObject::set(const zeno::Vector<zany2>& arr) {
    //    m_impl->set(zeVec2stdVec(arr));
    //}

    void ListObject::set(size_t index, IObject2* detached_obj) {
        if (0 > index || index >= m_objects.size())
            return;
        m_objects[index] = zany2(detached_obj);
    }

    void ListObject::set_obj(size_t index, zany2&& obj) {
        if (0 > index || index >= m_objects.size())
            return;
        m_objects[index] = std::move(obj);
    }


    std::unique_ptr<ListObject> create_ListObject() {
        return std::make_unique<ListObject>();
    }

    std::unique_ptr<ListObject> create_ListObject(std::vector<zany2>&& arrin) {
        auto pList = std::make_unique<ListObject>();
        for (auto& obj : arrin) {
            pList->m_objects.emplace_back(zany2(obj->clone()));
        }
        return pList;
    }
}
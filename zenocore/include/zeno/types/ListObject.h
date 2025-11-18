#pragma once

#include <zeno/utils/api.h>
#include <zeno/core/IObject.h>

namespace zeno {

struct ListObject_impl;

struct ZENO_API ListObject : IObjectClone<ListObject> {
    typedef IObjectClone<ListObject> base;

    ListObject();
    ListObject(const ListObject& rhs);
    ~ListObject();

    std::unique_ptr<IObject> clone() const override;
    void Delete() override;
    size_t size();
    void resize(size_t sz);
    void clear();
    IObject* get(int index);
    zany move(int index);
    zeno::ZsVector<IObject*> get();
    void push_back(zany&& obj);
    //void set(const zeno::Vector<zany>& arr);
    void set(size_t index, zany&& obj);
    void update_key(const String& key) override;
    bool has_change_info() const;
    bool empty() const;

    std::vector<int> get2_int() const;
    std::vector<float> get2_float() const;
    std::vector<std::string> get2_string() const;

    template <class T>
    std::vector<T*> getRaw() {
        const zeno::ZsVector<IObject*>& _vecList = get();
        std::vector<T*> resList;
        for (int i = 0; i < _vecList.size(); i++) {
            if (auto _obj = dynamic_cast<T*>(_vecList[i])) {
                resList.push_back(_obj);
            }
        }
        return resList;
    }

    std::unique_ptr<ListObject_impl> m_impl;
};

ZENO_API std::unique_ptr<ListObject> create_ListObject();
ZENO_API std::unique_ptr<ListObject> create_ListObject(std::vector<zany>&& arrin);

}

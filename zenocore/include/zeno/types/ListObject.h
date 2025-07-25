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
    zeno::SharedPtr<IObject> clone() const override;
    void Delete() override;
    size_t size();
    zany get(int index);
    zeno::Vector<zany> get();
    void push_back(zany&& obj);
    void push_back(const zany& obj);
    void set(const zeno::Vector<zany>& arr);
    void set(size_t index, zany obj);
    void update_key(const String& key) override;

    std::unique_ptr<ListObject_impl> m_impl;
};

ZENO_API zeno::SharedPtr<ListObject> create_ListObject();
ZENO_API zeno::SharedPtr<ListObject> create_ListObject(zeno::Vector<zany> arrin);

}

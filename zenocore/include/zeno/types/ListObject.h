#pragma once

#include <zeno/utils/api.h>
#include <zeno/core/IObject.h>

namespace zeno {

struct ListObject_impl;

struct ZENO_API ListObject : IObjectClone<ListObject> {

    void Delete() override;

    ListObject_impl* m_impl;
};

ZENO_API zeno::SharedPtr<ListObject> create_ListObject();
ZENO_API zeno::SharedPtr<ListObject> create_ListObject(zeno::Vector<zany> arrin);

}

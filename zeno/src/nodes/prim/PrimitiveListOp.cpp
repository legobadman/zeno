#include <zeno/zeno.h>
#include <zeno/types/PrimitiveObject.h>
#include <zeno/types/NumericObject.h>
#include <zeno/types/ListObject_impl.h>
#include <zeno/utils/safe_dynamic_cast.h>
#include <zeno/utils/vec.h>
#include <cstring>
#include <cstdlib>
#include <cassert>

namespace zeno {

struct MakePrimitiveFromList : zeno::INode {
  virtual void apply() override {
    auto prim = std::make_shared<PrimitiveObject>();
    auto list = ZImpl(get_input<ListObject>("list"));
    for (auto const &val: list->m_impl->getLiterial<vec3f>()) {
        prim->verts.push_back(val);
    }
    prim->verts.update();
    ZImpl(set_output("prim", std::move(prim)));
  }
};

ZENDEFNODE(MakePrimitiveFromList,
    { /* inputs: */ {
        {gParamType_List, "list", "", zeno::Socket_ReadOnly},
    }, /* outputs: */ {
        {gParamType_Primitive, "prim"},
    }, /* params: */ {
    }, /* category: */ {
    "primitive",
    }});

}

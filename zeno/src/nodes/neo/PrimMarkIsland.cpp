#include <zeno/zeno.h>
#include <zeno/types/PrimitiveObject.h>
#include <zeno/funcs/PrimitiveUtils.h>
#include <zeno/types/StringObject.h>
#include <zeno/types/NumericObject.h>
#include <zeno/utils/tuple_hash.h>
#include <unordered_map>

namespace zeno {

namespace {

struct PrimMarkIsland : INode {
    virtual void apply() override {
        auto prim = ZImpl(get_input<PrimitiveObject>("prim"));
        auto tagAttr = ZImpl(get_input<StringObject>("tagAttr"))->get();

        primMarkIsland(prim.get(), tagAttr);

        ZImpl(set_output("prim", std::move(prim)));
    }
};


ZENDEFNODE(PrimMarkIsland, {
    {
    {gParamType_Primitive, "prim", "", zeno::Socket_ReadOnly},
    {gParamType_String, "tagAttr", "tag"},
    },
    {
    {gParamType_Primitive, "prim"},
    },
    {
    },
    {"primitive"},
});

}
}

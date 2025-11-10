#include <zeno/zeno.h>
#include <zeno/types/PrimitiveObject.h>
#include <zeno/types/PrimitiveUtils.h>
#include <zeno/types/StringObject.h>
#include <zeno/types/NumericObject.h>
#include <zeno/utils/variantswitch.h>
#include <zeno/utils/logger.h>
#include <unordered_map>
#include <cassert>

namespace zeno {

struct PrimitiveLineSort : zeno::INode {
    virtual void apply() override {
        auto prim = ZImpl(get_input<zeno::PrimitiveObject>("prim"));
        primLineSort(prim.get(), ZImpl(get_param<bool>("reversed")));
        ZImpl(set_output("prim", std::move(prim)));
    }
};

ZENDEFNODE(PrimitiveLineSort, {
    {
    {gParamType_Primitive, "prim", "", zeno::Socket_ReadOnly},
    },
    {
    {gParamType_Primitive, "prim"},
    },
    {
    {gParamType_Bool, "reversed", "0"},
    },
    {"primitive"},
});

}

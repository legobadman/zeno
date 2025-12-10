#include <algorithm>
#include <zeno/zeno.h>
#include <zeno/types/PrimitiveTools.h>
#include <zeno/types/ListObject_impl.h>
#include <zeno/utils/vec.h>
#include <cstring>
#include <cstdlib>
#include <cassert>
#if defined(_OPENMP)
#include <omp.h>
#endif

namespace zeno {

struct PrimitiveMerge : zeno::INode {
  virtual void apply() override {
    auto list = ZImpl(get_input<ListObject>("listPrim"));
    if(!ZImpl(has_input("dst"))){
        auto outprim = primitive_merge(list.get());
        ZImpl(set_output("prim", std::move(outprim)));
    }
    else
    { // dage, weishenme buyong Assign jiedian ne?
        auto dst = ZImpl(get_input<PrimitiveObject>("dst"));
        auto outprim = primitive_merge(list.get());
        *dst = *outprim;
        ZImpl(set_output("prim", std::move(dst)));
    }
  }
};

ZENDEFNODE(PrimitiveMerge, {
    {
        {gParamType_Primitive, "dst", "", zeno::Socket_ReadOnly},
        {gParamType_List, "listPrim", "", zeno::Socket_ReadOnly},
    },
    {{gParamType_Primitive, "prim"}},
    {},
    {"deprecated"},
});


}

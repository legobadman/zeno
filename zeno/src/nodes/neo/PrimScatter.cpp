#include <zeno/zeno.h>
#include <zeno/funcs/PrimitiveUtils.h>
#include <zeno/types/PrimitiveObject.h>
#include <zeno/types/StringObject.h>
#include <zeno/types/NumericObject.h>
#include <zeno/para/parallel_for.h>
#include <zeno/para/parallel_scan.h>
#define ZENO_NOTICKTOCK
#include <zeno/utils/ticktock.h>
#include <zeno/utils/variantswitch.h>
#include <zeno/utils/wangsrng.h>
#include <zeno/utils/tuple_hash.h>
#include <zeno/utils/log.h>
#include <unordered_map>
#include <random>
#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace zeno {

template <class T>
static void revamp_vector(std::vector<T> &arr, std::vector<int> const &revamp) {
    std::vector<T> newarr(arr.size());
    for (int i = 0; i < revamp.size(); i++) {
        newarr[i] = arr[revamp[i]];
    }
    std::swap(arr, newarr);
}

namespace {

struct PrimScatter : INode {
    virtual void apply() override {
        auto prim = ZImpl(get_input<PrimitiveObject>("prim"));
        auto type = ZImpl(get_input2<std::string>("type"));
        auto denAttr = ZImpl(get_input2<std::string>("denAttr"));
        auto density = ZImpl(get_input2<float>("density"));
        auto minRadius = ZImpl(get_input2<float>("minRadius"));
        auto interpAttrs = ZImpl(get_input2<bool>("interpAttrs"));
        auto seed = ZImpl(get_input2<int>("seed"));
        auto retprim = primScatter(prim.get(), type, denAttr, density, minRadius, interpAttrs, seed);
        ZImpl(set_output("parsPrim", std::move(retprim)));
    }
};

ZENO_DEFNODE(PrimScatter)({
    {
        {gParamType_Primitive, "prim", "", zeno::Socket_ReadOnly},
        {"enum tris lines", "type", "tris"},
        {gParamType_String, "denAttr", ""},
        {gParamType_Float, "density", "100"},
        {gParamType_Float, "minRadius", "0"},
        {gParamType_Bool, "interpAttrs", "1"},
        {gParamType_Int, "seed", "-1"},
    },
    {
        {gParamType_Primitive, "parsPrim"},
    },
    {},
    {"primitive"},
});

}

}

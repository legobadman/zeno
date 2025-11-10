#include <zeno/zeno.h>
#include <zeno/types/PrimitiveObject.h>
#include <zeno/funcs/PrimitiveUtils.h>
//#include <zeno/para/execution.h>
//#include <algorithm>

namespace zeno {

#if 0
static void primSmoothNormal(PrimitiveObject *prim) {
    auto &nrm = prim->verts.add_attr<zeno::vec3f>("nrm");
    std::fill(ZENO_PAR_UNSEQ nrm.begin(), nrm.end(), vec3f());
    for (size_t i = 0; i < prim->polys.size(); i++) {
        auto [base, len] = prim->polys[i];
        if (len < 3) continue;
        auto a = prim->verts[prim->loops[base]];
        auto b = prim->verts[prim->loops[base + 1]];
        auto c = prim->verts[prim->loops[base + 2]];
        auto n = cross(b - a, c - a);
        for (size_t j = base + 2; j < base + len; j++) {
            auto b = prim->verts[prim->loops[j]];
            auto c = prim->verts[prim->loops[j + 1]];
            n += cross(b - a, c - a);
        }
        for (size_t j = base; j < base + len; j++) {
            nrm[j] += n;
        }
    }
    std::for_each(ZENO_PAR_UNSEQ nrm.begin(), nrm.end(), [] (vec3f &n) {
        n = normalizeSafe(n, 1e-6f);
    });
}
#endif

namespace {

struct PrimSepTriangles : INode {
    virtual void apply() override {
        auto prim = ZImpl(get_input<PrimitiveObject>("prim"));
        auto smoothNormal = ZImpl(get_input2<bool>("smoothNormal"));
        auto keepTriFaces = ZImpl(get_input2<bool>("keepTriFaces"));
        //printf("asdasd %d\n", prim->verts.attrs.erase("nrm"));
        primSepTriangles(prim.get(), smoothNormal, keepTriFaces);
        ZImpl(set_output("prim", std::move(prim)));
    }
};

ZENDEFNODE(PrimSepTriangles,
        { /* inputs: */ {
        {gParamType_Primitive, "prim", "", zeno::Socket_ReadOnly},
        {gParamType_Bool, "smoothNormal", "1"},
        {gParamType_Bool, "keepTriFaces", "1"},
        }, /* outputs: */ {
        {gParamType_Primitive, "prim"},
        }, /* params: */ {
        }, /* category: */ {
        "primitive",
        }});

}
}

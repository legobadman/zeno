#include <zeno/zeno.h>
#include <zeno/types/PrimitiveObject.h>
#include <zeno/funcs/PrimitiveUtils.h>
#include <zeno/geo/commonutil.h>
#include <zeno/types/StringObject.h>
#include <zeno/types/NumericObject.h>
#include <zeno/utils/tuple_hash.h>
#include <map>
#include <set>

namespace zeno {

ZENO_API void primEdgeBound(PrimitiveObject *prim, bool removeFaces, bool toEdges) {
    struct segment_less {
        bool operator()(vec2i const &a, vec2i const &b) const {
            return std::make_pair(std::min(a[0], a[1]), std::max(a[0], a[1]))
                < std::make_pair(std::min(b[0], b[1]), std::max(b[0], b[1]));
        }
    };
    std::map<vec2i, bool, segment_less> segments;
    auto append = [&] (int i, int j) {
        auto [it, succ] = segments.emplace(vec2i(i, j), false);
        if (!succ) {
            it->second = true;
        }
    };
    for (auto const &ind: prim->lines) {
        append(ind[0], ind[1]);
    }
    for (auto const &ind: prim->tris) {
        append(ind[0], ind[1]);
        append(ind[1], ind[2]);
        append(ind[2], ind[0]);
    }
    for (auto const &ind: prim->quads) {
        append(ind[0], ind[1]);
        append(ind[1], ind[2]);
        append(ind[2], ind[3]);
        append(ind[3], ind[0]);
    }
    for (auto const &[start, len]: prim->polys) {
        if (len < 2)
            continue;
        for (int i = start + 1; i < start + len; i++) {
            append(prim->loops[i - 1], prim->loops[i]);
        }
        append(prim->loops[start + len - 1], prim->loops[start]);
    }
    if (toEdges) {
        prim->edges.attrs.clear();
        prim->edges.clear();
        for (auto const &[k, v]: segments) {
            if (!v) prim->edges.push_back(k);
        }
        prim->edges.update();
    } else {
        prim->lines.attrs.clear();
        for (auto const &[k, v]: segments) {
            if (!v) prim->lines.push_back(k);
        }
        prim->lines.update();
    }
    if (removeFaces) {
        prim->tris.clear();
        prim->quads.clear();
        prim->loops.clear();
        prim->polys.clear();
    }
}



namespace {

struct PrimEdgeBound : INode {
    virtual void apply() override {
        auto prim = ZImpl(get_input<PrimitiveObject>("prim"));
        primEdgeBound(prim.get(), ZImpl(get_input2<bool>("removeFaces")), false);
        if (ZImpl(get_input2<bool>("killDeadVerts")))
            primKillDeadVerts(prim.get());
        ZImpl(set_output("prim", std::move(prim)));
    }
};

ZENDEFNODE(PrimEdgeBound, {
    {
    {gParamType_Primitive, "prim", "", zeno::Socket_ReadOnly},
    {gParamType_Bool, "removeFaces", "1"},
    {gParamType_Bool, "killDeadVerts", "1"},
    },
    {
    {gParamType_Primitive, "prim"},
    },
    {
    },
    {"primitive"},
});

struct PrimWireframe : INode {
    virtual void apply() override {
        auto prim = ZImpl(get_input<PrimitiveObject>("prim"));
        primWireframe(prim.get(), ZImpl(get_input2<bool>("removeFaces")), false);
        ZImpl(set_output("prim", std::move(prim)));
    }
};

ZENDEFNODE(PrimWireframe, {
    {
    {gParamType_Primitive, "prim", "", zeno::Socket_ReadOnly},
    {gParamType_Bool, "removeFaces", "1"},
    },
    {
    {gParamType_Primitive, "prim"},
    },
    {
    },
    {"primitive"},
});

struct PrimitiveWireframe : INode {
    virtual void apply() override {
        auto prim = ZImpl(get_input<PrimitiveObject>("prim"));
        primWireframe(prim.get(), ZImpl(get_param<bool>("removeFaces")), false);
        ZImpl(set_output("prim", std::move(prim)));
    }
};

ZENDEFNODE(PrimitiveWireframe, {
    {
    {gParamType_Primitive, "prim", "", zeno::Socket_ReadOnly},
    },
    {
    {gParamType_Primitive, "prim"},
    },
    {
    {gParamType_Bool, "removeFaces", "1"},
    },
    {"deprecated"},
});

}

}

#include <zeno/zeno.h>
#include <zeno/types/PrimitiveObject.h>
#include <zeno/funcs/PrimitiveUtils.h>
#include <zeno/types/NumericObject.h>
#include <zeno/types/StringObject.h>
#include <zeno/utils/vec.h>
#include <zeno/utils/zeno_p.h>
#include <zeno/geo/commonutil.h>
#include <cstring>
#include <cstdlib>

namespace zeno {

template <class T>
static void revamp_vector(std::vector<T> &arr, std::vector<int> const &revamp) {
    std::vector<T> newarr(revamp.size());
#pragma omp parallel for
    for (int i = 0; i < revamp.size(); i++) {
        newarr[i] = arr[revamp[i]];
    }
    std::swap(arr, newarr);
}

void primKillDeadUVs(PrimitiveObject *prim) {
    if (!(prim->loops.size() > 0 && prim->loops.attr_is<int>("uvs"))) {
        return;
    }
    auto &uvs = prim->loops.attr<int>("uvs");
    std::set<int> reached;
    for (auto const &[start, len]: prim->polys) {
        for (int i = start; i < start + len; i++) {
            reached.insert(uvs[i]);
        }
    }
    std::vector<int> revamp(reached.begin(), reached.end());
    auto old_prim_size = prim->uvs.size();
    prim->uvs.forall_attr<AttrAcceptAll>([&] (auto const &key, auto &arr) {
        revamp_vector(arr, revamp);
    });
    prim->uvs.resize(revamp.size());

    std::vector<int> unrevamp(old_prim_size);
    for (int i = 0; i < revamp.size(); i++) {
        unrevamp[revamp[i]] = i;
    }

    auto mock = [&] (int &ind) {
        ind = unrevamp[ind];
    };
    for (auto const &[start, len]: prim->polys) {
        for (int i = start; i < start + len; i++) {
            mock(uvs[i]);
        }
    }
}
void primKillDeadLoops(PrimitiveObject *prim) {
    if (prim->loops.size() == 0) {
        return;
    }
    std::set<int> reached;
    for (auto const &[start, len]: prim->polys) {
        for (int i = start; i < start + len; i++) {
            reached.insert(i);
        }
    }
    std::vector<int> revamp(reached.begin(), reached.end());
    auto old_prim_size = prim->loops.size();
    prim->loops.forall_attr<AttrAcceptAll>([&] (auto const &key, auto &arr) {
        revamp_vector(arr, revamp);
    });
    prim->loops.resize(revamp.size());

    std::vector<int> unrevamp(old_prim_size);
    for (int i = 0; i < revamp.size(); i++) {
        unrevamp[revamp[i]] = i;
    }
    int count = 0;
    for (auto &[start, len]: prim->polys) {
        start = count;
        count += len;
    }
}

ZENO_API void primKillDeadVerts(PrimitiveObject *prim) {
    std::set<int> reached(prim->points.begin(), prim->points.end());
    for (auto const &ind: prim->lines) {
        reached.insert(ind[0]);
        reached.insert(ind[1]);
    }
    for (auto const &ind: prim->tris) {
        reached.insert(ind[0]);
        reached.insert(ind[1]);
        reached.insert(ind[2]);
    }
    for (auto const &ind: prim->quads) {
        reached.insert(ind[0]);
        reached.insert(ind[1]);
        reached.insert(ind[2]);
        reached.insert(ind[3]);
    }
    for (auto const &[start, len]: prim->polys) {
        for (int i = start; i < start + len; i++) {
            reached.insert(prim->loops[i]);
        }
    }
    std::vector<int> revamp(reached.begin(), reached.end());
    auto old_prim_size = prim->verts.size();
    prim->verts.forall_attr<AttrAcceptAll>([&] (auto const &key, auto &arr) {
        revamp_vector(arr, revamp);
    });
    prim->verts.resize(revamp.size());

    std::vector<int> unrevamp(old_prim_size);
    for (int i = 0; i < revamp.size(); i++) {
        unrevamp[revamp[i]] = i;
    }

    auto mock = [&] (int &ind) {
        ind = unrevamp[ind];
    };
    for (auto &ind: prim->points) {
        mock(ind);
    }
    for (auto &ind: prim->lines) {
        mock(ind[0]);
        mock(ind[1]);
    }
    for (auto &ind: prim->tris) {
        mock(ind[0]);
        mock(ind[1]);
        mock(ind[2]);
    }
    for (auto &ind: prim->quads) {
        mock(ind[0]);
        mock(ind[1]);
        mock(ind[2]);
        mock(ind[3]);
    }
    for (auto const &[start, len]: prim->polys) {
        for (int i = start; i < start + len; i++) {
            mock(prim->loops[i]);
        }
    }
    primKillDeadUVs(prim);
    primKillDeadLoops(prim);
}

namespace {

struct PrimFilter : INode {
  virtual void apply() override {
    auto prim = ZImpl(get_input<PrimitiveObject>("prim"));
    auto tagAttr = ZImpl(get_input<StringObject>("tagAttr"))->get();
    auto revampAttrO = ZImpl(get_input<StringObject>("revampAttrO"))->get();
    auto tagValue = ZImpl(get_input<NumericObject>("tagValue"))->get<int>();
    auto isInversed = ZImpl(get_input<NumericObject>("isInversed"))->get<bool>();
    auto method = ZImpl(get_input<StringObject>("method"))->get();
    primFilterVerts(prim.get(), tagAttr, tagValue, isInversed, revampAttrO, method);
    ZImpl(set_output("prim", ZImpl(clone_input("prim"))));
  }
};

ZENDEFNODE(PrimFilter, {
    {
    {gParamType_Primitive, "prim", "", zeno::Socket_ReadOnly},
    {gParamType_String, "tagAttr", "tag"},
    {gParamType_String, "revampAttrO", ""},
    {gParamType_Int, "tagValue", "0"},
    {gParamType_Bool, "isInversed", "1"},
    {"enum verts faces", "method", "verts"},
    },
    {
    {gParamType_Primitive, "prim"},
    },
    {
    },
    {"primitive"},
});

struct PrimKillDeadVerts : INode {
    virtual void apply() override {
        auto prim = ZImpl(get_input<PrimitiveObject>("prim"));
        primKillDeadVerts(prim.get());
        ZImpl(set_output("prim", std::move(prim)));
    }
};

ZENDEFNODE(PrimKillDeadVerts, {
    {
    {gParamType_Primitive, "prim", "", zeno::Socket_ReadOnly},
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

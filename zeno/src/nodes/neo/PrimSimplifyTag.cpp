#include <zeno/zeno.h>
#include <zeno/types/PrimitiveObject.h>
#include <zeno/funcs/PrimitiveUtils.h>
#include <zeno/types/StringObject.h>
#include <zeno/types/NumericObject.h>
#include <unordered_map>
#include <random>

namespace zeno {

ZENO_API void primSimplifyTag(PrimitiveObject *prim, std::string tagAttr) {
    auto &tag = prim->verts.attr<int>(tagAttr);
    std::unordered_map<int, int> lut;
    int top = 0;
    for (int i = 0; i < tag.size(); i++) {
        auto k = tag[i];
        auto it = lut.find(k);
        if (it != lut.end()) {
            tag[i] = it->second;
        } else {
            int c = top++;
            lut.emplace(k, c);
            tag[i] = c;
        }
    }
}

ZENO_API void primColorByTag(PrimitiveObject *prim, std::string tagAttr, std::string clrAttr, int seed) {
    auto const &tag = prim->verts.attr<int>(tagAttr);
    auto &clr = prim->verts.add_attr<zeno::vec3f>(clrAttr);
    std::unordered_map<int, vec3f> lut;
    std::mt19937 gen{seed == -1 ? std::random_device{}() : seed};
    std::uniform_real_distribution<float> unif(0.2f, 1.0f);
    for (int i = 0; i < tag.size(); i++) {
        auto k = tag[i];
        auto it = lut.find(k);
        if (it != lut.end()) {
            clr[i] = it->second;
        } else {
            vec3f c(unif(gen), unif(gen), unif(gen));
            lut.emplace(k, c);
            clr[i] = c;
        }
    }
}


namespace {

struct PrimSimplifyTag : INode {
    virtual void apply() override {
        auto prim = ZImpl(get_input<PrimitiveObject>("prim"));
        auto tagAttr = ZImpl(get_input<StringObject>("tagAttr"))->get();

        primSimplifyTag(prim.get(), tagAttr);

        ZImpl(set_output("prim", std::move(prim)));
    }
};


ZENDEFNODE(PrimSimplifyTag, {
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

struct PrimColorByTag : INode {
    virtual void apply() override {
        auto prim = ZImpl(get_input<PrimitiveObject>("prim"));
        auto tagAttr = ZImpl(get_input<StringObject>("tagAttr"))->get();
        auto clrAttr = ZImpl(get_input<StringObject>("clrAttr"))->get();
        auto seed = ZImpl(get_input<NumericObject>("seed"))->get<int>();

        primColorByTag(prim.get(), tagAttr, clrAttr, seed);

        ZImpl(set_output("prim", std::move(prim)));
    }
};


ZENDEFNODE(PrimColorByTag, {
    {
    {gParamType_Primitive, "prim", "", zeno::Socket_ReadOnly},
    {gParamType_String, "tagAttr", "tag"},
    {gParamType_String, "clrAttr", "clr"},
    {gParamType_Int, "seed", "-1"},
    },
    {
    {gParamType_Primitive, "prim"},
    },
    {
    },
    {"visualize"},
});

}
}

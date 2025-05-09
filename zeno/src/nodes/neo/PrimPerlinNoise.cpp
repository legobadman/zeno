#include <zeno/zeno.h>
#include <zeno/types/StringObject.h>
#include <zeno/types/PrimitiveObject.h>
#include <zeno/funcs/PrimitiveUtils.h>
#include <zeno/types/NumericObject.h>
#include <zeno/utils/variantswitch.h>
#include <zeno/utils/arrayindex.h>
#include <zeno/para/parallel_for.h>
#include <zeno/utils/perlin.h>
#include <zeno/utils/vec.h>
#include <zeno/utils/log.h>
#include <cstring>
#include <cstdlib>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace zeno {

ZENO_API void primPerlinNoise(PrimitiveObject *prim, std::string inAttr, std::string outAttr, std::string outType, float scale, float detail, float roughness, float disortion, vec3f offset, float average, float strength) {
    prim->attr_visit(inAttr, [&] (auto const &inArr) {
        std::visit([&] (auto outTypeId) {
            using InT = std::decay_t<decltype(inArr[0])>;
            using OutT = decltype(outTypeId);
            auto &outArr = prim->add_attr<OutT>(outAttr);
            parallel_for((size_t)0, inArr.size(), [&] (size_t i) {
                vec3f p;
                InT inp = inArr[i];
                if constexpr (std::is_same_v<InT, float>) {
                    p = {inp, 0, 0};
                } else if constexpr (std::is_same_v<InT, int>) {
                    p = {(float)inp, 0, 0};
                } else if constexpr (std::is_same_v<InT, vec2f>) {
                    p = {inp[0], inp[1], 0};
                } else if constexpr (std::is_same_v<InT, vec3f>) {
                    p = inp;
                } else if constexpr (std::is_same_v<InT, vec4f>) {
                    p = {inp[0], inp[1], inp[2]};
                } else {
                    throw makeError<TypeError>(typeid(vec3f), typeid(InT), "input type");
                }
                p = scale * (p - offset);
                OutT o;
                if constexpr (std::is_same_v<OutT, float>) {
                    o = PerlinNoise::perlin(p, roughness, detail);
                } else if constexpr (std::is_same_v<OutT, vec3f>) {
                    o = {
                        PerlinNoise::perlin(vec3f(p[0], p[1], p[2]), roughness, detail),
                        PerlinNoise::perlin(vec3f(p[1], p[2], p[0]), roughness, detail),
                        PerlinNoise::perlin(vec3f(p[2], p[0], p[1]), roughness, detail),
                    };
                } else {
                    throw makeError<TypeError>(typeid(vec3f), typeid(OutT), "outType");
                }
                o = average + o * strength;
                outArr[i] = o;
            });
        }, enum_variant<std::variant<float, vec3f>>(array_index_safe({"float", "vec3f"}, outType, "outType")));
    });
}

namespace {

struct PrimPerlinNoise : INode {
    virtual void apply() override {
        auto prim = ZImpl(get_input<PrimitiveObject>("prim"));
        auto scale = ZImpl(get_input2<float>("scale"));
        auto detail = ZImpl(get_input2<float>("detail"));
        auto roughness = ZImpl(get_input2<float>("roughness"));
        auto disortion = ZImpl(get_input2<float>("disortion"));
        auto offset = ZImpl(get_input2<zeno::vec3f>("offset"));
        auto average = ZImpl(get_input2<float>("average"));
        auto strength = ZImpl(get_input2<float>("strength"));
        auto outAttr = ZImpl(get_input2<std::string>("outAttr"));
        auto inAttr = ZImpl(get_input2<std::string>("inAttr"));
        auto outType = ZImpl(get_input2<std::string>("outType"));
        primPerlinNoise(prim.get(), inAttr, outAttr, outType, scale,
                        detail, roughness, disortion, offset, average, strength);
        ZImpl(set_output("prim", ZImpl(get_input("prim"))));
    }
};

ZENDEFNODE(PrimPerlinNoise, {
    {
    {gParamType_Primitive, "prim", "", zeno::Socket_ReadOnly},
    {gParamType_String, "inAttr", "pos"},
    {gParamType_String, "outAttr", "tmp"},
    {gParamType_Float, "scale", "5"},
    {gParamType_Float, "detail", "2"},
    {gParamType_Float, "roughness", "0.5"},
    {gParamType_Float, "disortion", "0"},
    {gParamType_Vec3f, "offset", "0,0,0"},
    {gParamType_Float, "average", "0"},
    {gParamType_Float, "strength", "1"},
    {"enum float vec3f", "outType", "float"},
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

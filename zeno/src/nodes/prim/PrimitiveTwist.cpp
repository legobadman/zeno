#include <zeno/zeno.h>
#include <zeno/types/PrimitiveObject.h>
#include <zeno/types/StringObject.h>
#include <zeno/types/NumericObject.h>
#include <zeno/utils/orthonormal.h>
#include <zeno/utils/parallel_reduce.h>
#include <sstream>
#include <iostream>
#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace zeno {

struct PrimitiveTwist : zeno::INode { // todo: also add PrimitiveStretch and PrimitiveTaper
    virtual void apply() override {
        auto prim = ZImpl(get_input<zeno::PrimitiveObject>("prim"));
        auto angle = ZImpl(get_input<zeno::NumericObject>("angle"))->get<float>();
        auto limitMin = ZImpl(get_input<zeno::NumericObject>("limitMin"))->get<float>();
        auto limitMax = ZImpl(get_input<zeno::NumericObject>("limitMax"))->get<float>();
        limitMin = std::min(1.f, std::max(0.f, limitMin));
        limitMax = std::min(1.f, std::max(0.f, limitMax));
        limitMin -= 0.5f;
        limitMax -= 0.5f;

        auto origin = ZImpl(has_input("origin")) ? ZImpl(get_input<zeno::NumericObject>("origin"))->get<zeno::vec3f>() : vec3f(0, 0, 0);
        auto direction = ZImpl(has_input("direction")) ? ZImpl(get_input<zeno::NumericObject>("direction"))->get<zeno::vec3f>() : vec3f(0, 1, 0);

        auto orb = ZImpl(has_input("tangent"))
            ? orthonormal(direction, ZImpl(get_input<zeno::NumericObject>("tangent"))->get<zeno::vec3f>())
            : orthonormal(direction);
        direction = orb.normal;
        auto tangent = orb.tangent;
        auto bitangent = orb.bitangent;

        auto restx = vec3f(1, 1, 1);

        if (std::abs(angle) > 0.005f && limitMax - limitMin > 0.001f) {
            angle *= M_PI / 180;
            angle /= limitMax - limitMin;

            //printf("tangent: %f %f %f\n", tangent[0], tangent[1], tangent[2]);
            //printf("bitangent: %f %f %f\n", bitangent[0], bitangent[1], bitangent[2]);

            auto acc = parallel_reduce_array(prim->size(), vec2f(prim->size() ? dot(direction, prim->verts[0] - origin) : 0.f), [&] (size_t i) {
                return vec2f(dot(direction, prim->verts[i] - origin));
            }, [&] (auto a, auto b) { return vec2f(std::min(a[0], b[0]), std::max(a[1], b[1])); });
            auto height = acc[1] - acc[0];
            auto middle = (acc[1] + acc[0]) * 0.5f;
            auto inv_height = 1 / height;

#pragma omp parallel for
            for (intptr_t i = 0; i < prim->verts.size(); i++) {
                auto pos = prim->verts[i] - origin;

                auto dirpos = dot(pos, direction);
                auto fac = (dirpos - middle) * inv_height;
                auto ang = std::max(limitMin, std::min(fac, limitMax)) * angle;
                auto sinang = std::sin(ang);
                auto cosang = std::cos(ang);

                auto tanpos = dot(pos, tangent);
                auto bitpos = dot(pos, bitangent);

                auto newtanpos = tanpos * cosang - bitpos * sinang;
                auto newbitpos = bitpos * cosang + tanpos * sinang;

                pos += (newtanpos - tanpos) * tangent + (newbitpos - bitpos) * bitangent;

                prim->verts[i] = pos + origin;
            }

        }
        ZImpl(set_output("prim", std::move(prim)));
    }
};


ZENDEFNODE(PrimitiveTwist, {
    {
    {gParamType_Primitive, "prim", "", zeno::Socket_ReadOnly},
    {gParamType_Vec3f, "origin", "0,0,0"},
    {gParamType_Vec3f, "direction", "0,1,0"},
    {gParamType_Vec3f, "tangent"},
    {gParamType_Float, "angle", "45"},
    {gParamType_Float, "limitMin", "0"},
    {gParamType_Float, "limitMax", "1"},
    },
    {
    {gParamType_Primitive, "prim"},
    },
    {
    },
    {"deprecated"},
});

}

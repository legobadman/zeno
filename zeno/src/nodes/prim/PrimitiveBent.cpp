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

struct PrimitiveBent : zeno::INode {
    virtual void apply() override {
        auto prim = ZImpl(get_input<zeno::PrimitiveObject>("prim"));
        auto angle = ZImpl(get_input<zeno::NumericObject>("angle"))->get<float>();
        auto limitMin = ZImpl(get_input<zeno::NumericObject>("limitMin"))->get<float>();
        auto limitMax = ZImpl(get_input<zeno::NumericObject>("limitMax"))->get<float>();
        auto midPoint = ZImpl(get_input<zeno::NumericObject>("midPoint"))->get<float>();
        auto biasDir = ZImpl(get_input<zeno::NumericObject>("biasDir"))->get<float>();
        limitMin = std::min(1.f, std::max(0.f, limitMin));
        limitMax = std::min(1.f, std::max(0.f, limitMax));
        if (limitMin > limitMax)
            std::swap(limitMin, limitMax);
        midPoint = std::min(limitMax, std::max(limitMin, midPoint));
        biasDir = std::min(1.f, std::max(0.f, biasDir));

        auto origin = ZImpl(has_input("origin")) ? ZImpl(get_input<zeno::NumericObject>("origin"))->get<zeno::vec3f>() : vec3f(0, 0, 0);
        auto tangent = ZImpl(has_input("tangent")) ? ZImpl(get_input<zeno::NumericObject>("tangent"))->get<zeno::vec3f>() : vec3f(0, 1, 0);
        auto direction = ZImpl(has_input("direction")) ? ZImpl(get_input<zeno::NumericObject>("direction"))->get<zeno::vec3f>() : vec3f(1, 0, 0);

        orthonormal orb(direction, tangent);
        direction = orb.normal;
        tangent = orb.tangent;

        if (std::abs(angle) > 0.005f && limitMax - limitMin > 0.001f && prim->size() != 0) {
            angle *= M_PI / 180;
            angle /= limitMax - limitMin;

            auto tanv0 = dot(tangent, prim->verts[0]);
            auto dirv0 = dot(direction, prim->verts[0]);
            auto acc = parallel_reduce_array(prim->size(), vec4f(tanv0, tanv0, dirv0, dirv0), [&] (size_t i) {
                auto tanv = dot(tangent, prim->verts[i]);
                auto dirv = dot(direction, prim->verts[i]);
                return vec4f(tanv, tanv, dirv, dirv);
            }, [&] (auto a, auto b) { return vec4f(std::min(a[0], b[0]), std::max(a[1], b[1]), std::min(a[2], b[2]), std::max(a[3], b[3])); });

            if (ZImpl(get_param<bool>("useOrigin"))) {
                biasDir = (dot(direction, origin) - acc[2]) / (acc[3] - acc[2]);
                midPoint = (dot(tangent, origin) - acc[0]) / (acc[1] - acc[0]);
            }

            auto height = acc[1] - acc[0];
            auto average = (acc[1] + acc[0]) * 0.5f;
            auto middle = acc[1] * midPoint + acc[0] * (1 - midPoint);
            auto truemid = height * (0.5f - midPoint);
            auto radius = height / angle;
            auto inv_height = 1 / height;

            auto avgDir = (acc[3] + acc[2]) * 0.5f;
            biasDir = (biasDir - 0.5f) * (acc[3] - acc[2]);
            limitMin -= midPoint;
            limitMax -= midPoint;

#pragma omp parallel for
            for (intptr_t i = 0; i < prim->verts.size(); i++) {
                auto pos = prim->verts[i] + (biasDir - avgDir) * direction;
                auto tanpos = dot(tangent, pos);
                auto dirpos = dot(direction, pos);
                auto fac = (tanpos - middle) * inv_height;

                auto rad = radius - dirpos;
                auto ang = std::max(limitMin, std::min(fac, limitMax)) * angle;
                auto sinang = std::sin(ang);
                auto cosang = std::cos(ang);
                auto newtanpos = rad * sinang;
                auto newdirpos = rad * cosang;

                auto diff = (std::max(0.f, limitMin - fac) + std::min(0.f, limitMax - fac)) * height;
                newtanpos -= diff * cosang;
                newdirpos += diff * sinang;

                newdirpos -= radius;
                pos += (newtanpos - tanpos + average) * tangent;
                pos += (biasDir + avgDir - newdirpos - dirpos) * direction;

                prim->verts[i] = pos;
            }

        }
        ZImpl(set_output("prim", std::move(prim)));
    }
};


ZENDEFNODE(PrimitiveBent, {
    {
    {gParamType_Primitive, "prim", "", zeno::Socket_ReadOnly},
    {gParamType_Vec3f, "origin", "0,0,0"},
    {gParamType_Vec3f, "tangent", "0,1,0"},
    {gParamType_Vec3f, "direction", "1,0,0"},
    {gParamType_Float, "angle", "45"},
    {gParamType_Float, "limitMin", "0"},
    {gParamType_Float, "limitMax", "1"},
    {gParamType_Float, "midPoint", "0.5"},
    {gParamType_Float, "biasDir", "0.5"},
    },
    {
    {gParamType_Primitive, "prim"},
    },
    {
    {gParamType_Bool, "useOrigin", "0"},
    },
    {"deprecated"},
});

}

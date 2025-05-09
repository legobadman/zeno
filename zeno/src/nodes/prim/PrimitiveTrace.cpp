#include <zeno/zeno.h>
#include <zeno/types/PrimitiveObject.h>
#include <zeno/types/NumericObject.h>
#include <zeno/utils/vec.h>
#include <cstring>
#include <cstdlib>
#include <cassert>

namespace zeno {


struct PrimitiveTraceTrail : zeno::INode {
    std::shared_ptr<PrimitiveObject> trailPrim = std::make_shared<PrimitiveObject>();

    virtual void apply() override {
        auto parsPrim = ZImpl(get_input<PrimitiveObject>("parsPrim"));

        int base = trailPrim->size();
        int last_base = base - parsPrim->size();
        trailPrim->resize(base + parsPrim->size());

        parsPrim->foreach_attr([&] (auto const &parsAttr, auto const &parsArr) {
            ([&, trailAttr = parsAttr] (auto const &parsArr) {
                using T = std::decay_t<decltype(parsArr[0])>;
                auto &trailArr = trailPrim->add_attr<T>(trailAttr);
                for (int i = 0; i < parsPrim->size(); i++) {
                    trailArr[base + i] = parsArr[i];
                }
            })(parsArr);
        });
        if (last_base > 0) {
            for (int i = 0; i < parsPrim->size(); i++) {
                trailPrim->lines.emplace_back(base + i, last_base + i);
            }
        }

        ZImpl(set_output("trailPrim", trailPrim));
    }
};

ZENDEFNODE(PrimitiveTraceTrail,
    { /* inputs: */ {
    {gParamType_Primitive, "parsPrim", "", zeno::Socket_ReadOnly},
    }, /* outputs: */ {
    {gParamType_Primitive, "trailPrim"},
    }, /* params: */ {
    }, /* category: */ {
    "primitive",
    }});


struct PrimitiveCalcVelocity : zeno::INode {
    std::vector<vec3f> last_pos;
    bool no_last_pos = true;

    virtual void apply() override {
        auto prim = ZImpl(get_input<PrimitiveObject>("prim"));
        auto dt = ZImpl(has_input("dt")) ? ZImpl(get_input<NumericObject>("dt"))->get<float>() : 0.04f;
        auto const &pos = prim->attr<zeno::vec3f>("pos");
        if (no_last_pos) {
            last_pos = pos;
            no_last_pos = false;
        }
        auto &vel = prim->add_attr<zeno::vec3f>("vel");

#pragma omp parallel for
        for (int i = 0; i < std::min(last_pos.size(), pos.size()); i++) {
            vel[i] = (pos[i] - last_pos[i]) / dt;
        }
        last_pos = pos;

        ZImpl(set_output("prim", std::move(prim)));
    }
};

ZENDEFNODE(PrimitiveCalcVelocity,
    { /* inputs: */ {
    {gParamType_Primitive, "prim", "", zeno::Socket_ReadOnly},
    {gParamType_Float, "dt", "0.04"},
    }, /* outputs: */ {
    {gParamType_Primitive, "prim"},
    }, /* params: */ {
    }, /* category: */ {
    "primitive",
    }});


struct PrimitiveInterpSubframe : zeno::INode {
    std::vector<vec3f> base_pos;
    std::vector<vec3f> curr_pos;

    virtual void apply() override {
        auto prim = ZImpl(get_input<PrimitiveObject>("prim"));
        auto portion = ZImpl(get_input<NumericObject>("portion"))->get<float>();

        if (portion == 0) {
            base_pos = std::move(curr_pos);
            curr_pos = prim->attr<zeno::vec3f>("pos");
        }

        auto &pos = prim->attr<zeno::vec3f>("pos");

#pragma omp parallel for
        for (int i = 0; i < std::min(pos.size(),
                    std::min(curr_pos.size(), base_pos.size())); i++) {
            pos[i] = mix(base_pos[i], curr_pos[i], portion);
        }

        ZImpl(set_output("prim", std::move(prim)));
    }
};

ZENDEFNODE(PrimitiveInterpSubframe,
    { /* inputs: */ {
    {gParamType_Primitive, "prim", "", zeno::Socket_ReadOnly},
    {gParamType_Float, "portion"},
    }, /* outputs: */ {
    {gParamType_Primitive, "prim"},
    }, /* params: */ {
    }, /* category: */ {
    "primitive",
    }});


}

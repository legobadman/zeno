#include <zeno/zeno.h>
#include <zeno/types/ParticlesObject.h>
#include <zeno/PrimitiveObject.h>
#include <zeno/NumericObject.h>
#include <zeno/utils/vec.h>
#include <cstring>
#include <cstdlib>
#include <cassert>

namespace zeno {

struct PrimitiveToParticles : zeno::INode{
    virtual void apply() override {
    auto prim = get_input("prim")->as<PrimitiveObject>();
    auto result = zeno::IObject::make<ParticlesObject>();
    result->pos.resize(prim->size());
    result->vel.resize(prim->size());

    #pragma omp parallel for
    for(int i=0;i<prim->size();i++)
    {
        result->pos[i] = zeno::vec_to_other<glm::vec3>(prim->attr<zeno::vec3f>("pos")[i]);
        if (prim->has_attr("vel"))
            result->vel[i] = zeno::vec_to_other<glm::vec3>(prim->attr<zeno::vec3f>("vel")[i]);
    }
    
    set_output("pars", result);
  }
};

static int defPrimitiveToParticles = zeno::defNodeClass<PrimitiveToParticles>("PrimitiveToParticles",
    { /* inputs: */ {
{gParamType_Primitive, "prim"},
}, /* outputs: */ {
        {"object", "pars"},
    }, /* params: */ { 
    }, /* category: */ {
    "deprecated",
    }});

}

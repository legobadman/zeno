#include <zeno/NumericObject.h>
#include <zeno/PrimitiveObject.h>
#include <zeno/StringObject.h>
#include <zeno/VDBGrid.h>
#include <zeno/utils/vec.h>
#include <zeno/zeno.h>
#include <zeno/ZenoInc.h>

namespace zeno {

struct TraceOneStep : INode {
  virtual void apply() override {
    auto steps = get_input2_int("steps");
    auto prim = get_input_PrimitiveObject("prim");
    auto vecField = safe_dynamic_cast<VDBFloat3Grid>(get_input("vecField"));
    auto size = get_input2_int("size");
    auto dt = get_input2_float("dt");
    auto maxlength = std::numeric_limits<float>::infinity();
    if(has_input("maxlength"))
    {
      maxlength = get_input2_float("maxlength");
    }
    
    for(auto s=0;s<steps;s++){
      prim->resize(prim->size()+size);
      auto &pos = prim->attr<vec3f>("pos");
      auto &lengtharr = prim->attr<float>("length");
      auto &velarr = prim->attr<vec3f>("vel");
      prim->lines.resize(prim->lines.size() + size);

      #pragma omp parallel for
      for(int i=prim->size()-size; i<prim->size(); i++)
      {
        auto p0 = pos[i-size];
        
        auto p1 = vec_to_other<openvdb::Vec3R>(p0);
        auto p2 = vecField->worldToIndex(p1);
        auto vel = openvdb::tools::BoxSampler::sample(vecField->m_grid->tree(), p2);
        velarr[i-size] = other_to_vec<3>(vel);
        auto pend = p0;
        if(lengtharr[i-size]<maxlength && maxlength>0)
        {
            pend += dt * other_to_vec<3>(vel);
        }
        pos[i] = pend;
        velarr[i] = velarr[i-size];
        lengtharr[i] = lengtharr[i-size] + length(pend - p0);
        prim->lines[i-size] = zeno::vec2i(i-size, i);
      }
    }
    set_output("prim", std::move(prim));
  }
};

ZENDEFNODE(TraceOneStep, {
                                     {
                                            {gParamType_Primitive, "prim"},
                                            {gParamType_Float, "dt"},
                                            {gParamType_Int, "size"},
                                            {gParamType_Int, "steps"},
                                            {gParamType_Float, "maxlength"},
                                            {gParamType_VDBGrid,"vecField"}
                                     },
                                     {
                                         {gParamType_Primitive, "prim"},
                                     },
                                     {},
                                     {"openvdb"},
                                 });
}

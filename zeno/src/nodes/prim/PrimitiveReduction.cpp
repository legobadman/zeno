#include <zeno/zeno.h>
#include <zeno/types/PrimitiveObject.h>
#include <zeno/types/NumericObject.h>
#include <zeno/utils/parallel_reduce.h>
#include <zeno/utils/vec.h>
#include <cstring>
#include <cstdlib>
#include <cassert>
//#include <tbb/parallel_for.h>
//#include <tbb/parallel_reduce.h>

namespace zeno {

template <class T>
static T prim_reduce(PrimitiveObject *prim, std::string channel, std::string type)
{
    std::vector<T> const &temp = prim->attr<T>(channel);
    
    if(type==std::string("avg")){
        T total = zeno::parallel_reduce_array<T>(temp.size(), T(0), [&] (size_t i) -> T { return temp[i]; },
        [&] (T i, T j) -> T { return i + j; });
        return total/(T)(temp.size());
    }
    if(type==std::string("max")){
        T total = zeno::parallel_reduce_array<T>(temp.size(), temp[0], [&] (size_t i) -> T { return temp[i]; },
        [&] (T i, T j) -> T { return zeno::max(i, j); });
        return total;   
    }
    if(type==std::string("min")){
        T total = zeno::parallel_reduce_array<T>(temp.size(), temp[0], [&] (size_t i) -> T { return temp[i]; },
        [&] (T i, T j) -> T { return zeno::min(i, j); });
        return total;
    }
    if(type==std::string("absmax")){
        T total = zeno::parallel_reduce_array<T>(temp.size(), temp[0], [&] (size_t i) -> T { return zeno::abs(temp[i]); },
        [&] (T i, T j) -> T { return zeno::max(i, j); });
        return total;
    }
    return T(0);
}


struct PrimitiveReduction : zeno::INode {
    virtual void apply() override{
        auto prim = ZImpl(get_input<PrimitiveObject>("prim"));
        auto attrToReduce = ZImpl(get_param<std::string>("attr"));
        auto op = ZImpl(get_param<std::string>("op"));
        zeno::NumericValue result;
        if (prim->attr_is<zeno::vec3f>(attrToReduce))
            result = prim_reduce<zeno::vec3f>(prim.get(), attrToReduce, op);
        else 
            result = prim_reduce<float>(prim.get(), attrToReduce, op);
        auto out = std::make_shared<zeno::NumericObject>();
        out->set(result);
        ZImpl(set_output("result", std::move(out)));
    }
};
ZENDEFNODE(PrimitiveReduction,
    { /* inputs: */ {
        {gParamType_Primitive, "prim", "", zeno::Socket_ReadOnly},
    }, /* outputs: */ {
        {gParamType_Vec3f, "result", ""},
    }, /* params: */ {
    {gParamType_String, "attr", "pos"},
    {"enum avg max min absmax", "op", "avg"},
    }, /* category: */ {
    "deprecated",
    }});

struct PrimReduction : zeno::INode {
    virtual void apply() override{
        auto prim = ZImpl(get_input<PrimitiveObject>("prim"));
        auto attrToReduce = ZImpl(get_input2<std::string>(("attrName")));
        auto op = ZImpl(get_input2<std::string>(("op")));
        zeno::NumericValue result;
        if (prim->attr_is<zeno::vec3f>(attrToReduce))
            result = prim_reduce<zeno::vec3f>(prim.get(), attrToReduce, op);
        else 
            result = prim_reduce<float>(prim.get(), attrToReduce, op);
        auto out = std::make_shared<zeno::NumericObject>();
        out->set(result);
        ZImpl(set_output("result", std::move(out)));
    }
};
ZENDEFNODE(PrimReduction,{
    {
        {gParamType_Primitive, "prim", "", zeno::Socket_ReadOnly},
        {gParamType_String, "attrName", "pos"},
        {"enum avg max min absmax", "op", "avg"},
    },
    {
        {"NumericObject","result"},
    },
    {},
    {"primitive"},
});

struct PrimitiveBoundingBox : zeno::INode {
    virtual void apply() override{
        auto prim = ZImpl(get_input<PrimitiveObject>("prim"));
        auto &pos = prim->attr<zeno::vec3f>("pos");

        auto bmin = pos.size() ? pos[0] : vec3f(0);
        auto bmax = bmin;

        for (int i = 1; i < pos.size(); i++) {
            bmin = zeno::min(pos[i], bmin);
            bmax = zeno::max(pos[i], bmax);
        }

        // auto exwidth = ZImpl(get_param<float>("exWidth"));
        auto exwidth = ZImpl(has_input("exWidth")) ? ZImpl(get_input2<float>("exWidth")) : 0.f;
        if (exwidth) {
            bmin -= exwidth;
            bmax += exwidth;
        }
        ZImpl(set_output2("bmin", bmin));
        ZImpl(set_output2("bmax", bmax));
    }
};

ZENDEFNODE(PrimitiveBoundingBox,
    { /* inputs: */ {
    {gParamType_Primitive, "prim", "", zeno::Socket_ReadOnly},
    {gParamType_Float, "exWidth", "0"},
    }, /* outputs: */ {
    {gParamType_Vec3f, "bmin"}, {gParamType_Vec3f, "bmax"},
    }, /* params: */ {
    }, /* category: */ {
    "primitive",
    }});

}

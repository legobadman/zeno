#include <zeno/zeno.h>
#include <zeno/types/StringObject.h>
#include <zeno/types/PrimitiveObject.h>
#include <zeno/funcs/PrimitiveUtils.h>
#include <zeno/utils/variantswitch.h>
#include <zeno/utils/arrayindex.h>
#include <zeno/para/parallel_for.h>
#include <zeno/utils/parallel_reduce.h>
#include <stdexcept>
#include <cmath>


namespace zeno {
namespace {

struct PrimFillAttr : INode {
    virtual void apply() override {
        auto prim = ZImpl(get_input<PrimitiveObject>("prim"));
        auto value = ZImpl(get_input<NumericObject>("value"));
        auto attr = ZImpl(get_input2<std::string>("attr"));
        auto type = ZImpl(get_input2<std::string>("type"));
        auto scope = ZImpl(get_input2<std::string>("scope"));
        std::visit([&] (auto ty) {
            using T = decltype(ty);
            auto val = value->get<T>();
            if (scope == "vert") {
            auto &arr = prim->verts.add_attr<T>(attr);
                std::fill(arr.begin(), arr.end(), val);
            }
            else if (scope == "tri") {
                auto &arr = prim->tris.add_attr<T>(attr);
                std::fill(arr.begin(), arr.end(), val);
            }
            else if (scope == "loop") {
                auto &arr = prim->loops.add_attr<T>(attr);
                std::fill(arr.begin(), arr.end(), val);
            }
            else if (scope == "poly") {
                auto &arr = prim->polys.add_attr<T>(attr);
                std::fill(arr.begin(), arr.end(), val);
            }
            else if (scope == "line") {
                auto &arr = prim->lines.add_attr<T>(attr);
                std::fill(arr.begin(), arr.end(), val);
            }
        }, enum_variant<std::variant<
            float
            , vec2f
            , vec3f
            , vec4f
            , int
            , vec2i
            , vec3i
            , vec4i
        >>(array_index({
            "float"
            , "vec2f"
            , "vec3f"
            , "vec4f"
            , "int"
            , "vec2i"
            , "vec3i"
            , "vec4i"
        }, type)));
        ZImpl(set_output("prim", std::move(prim)));
    }
};

ZENDEFNODE(PrimFillAttr, {
    {
    {gParamType_Primitive, "prim", "", zeno::Socket_ReadOnly},
        {"enum vert tri loop poly line", "scope", "vert"},
    {gParamType_String, "attr", "rad"},
        {"enum float vec2f vec3f vec4f int vec2i vec3i vec4i", "type", "float"},
    {gParamType_Float, "value", "0"},
    },
    {
    {gParamType_Primitive, "prim"},
    },
    {
    },
    {"primitive"},
});

struct PrimFillColor : PrimFillAttr {
    virtual void apply() override {
        ZImpl(set_primitive_input("attr", "clr"));
        ZImpl(set_primitive_input("type", "vec3f"));
        ZImpl(set_primitive_input("scope", "vert"));
        PrimFillAttr::apply();
    }
};

ZENDEFNODE(PrimFillColor, {
    {
    {gParamType_Primitive, "prim", "", zeno::Socket_ReadOnly},
    {gParamType_Vec3f, "value", "1,0.5,0.5"},
    },
    {
    {gParamType_Primitive, "prim"},
    },
    {
    },
    {"primitive"},
});

struct PrimFloatAttrToInt : INode {
    virtual void apply() override {
        auto prim = ZImpl(get_input<PrimitiveObject>("prim"));
        auto attr = ZImpl(get_input2<std::string>("attr"));
        auto attrOut = ZImpl(get_input2<std::string>("attrOut"));
        if(prim->verts.has_attr(attr)){
        auto &inArr = prim->verts.attr<float>(attr);
        auto factor = ZImpl(get_input2<float>("divisor"));
        if (attrOut == attr) {
            std::vector<int> outArr(inArr.size());
            parallel_for(inArr.size(), [&] (size_t i) {
                outArr[i] = std::rint(inArr[i] * factor);
            });
            prim->verts.attrs.erase(attrOut);
            prim->verts.add_attr<int>(attrOut) = std::move(outArr);
        } else {
            auto &outArr = prim->verts.add_attr<int>(attrOut);
            parallel_for(inArr.size(), [&] (size_t i) {
                outArr[i] = std::rint(inArr[i] * factor);
            });
        }
        }
        if(prim->tris.has_attr(attr)){
            auto &inArr = prim->tris.attr<float>(attr);
            auto factor = ZImpl(get_input2<float>("divisor"));
            if (attrOut == attr) {
                std::vector<int> outArr(inArr.size());
                parallel_for(inArr.size(), [&] (size_t i) {
                    outArr[i] = std::rint(inArr[i] * factor);
                });
                prim->tris.attrs.erase(attrOut);
                prim->tris.add_attr<int>(attrOut) = std::move(outArr);
            } else {
                auto &outArr = prim->tris.add_attr<int>(attrOut);
                parallel_for(inArr.size(), [&] (size_t i) {
                    outArr[i] = std::rint(inArr[i] * factor);
                });
            }
        }
        if(prim->polys.has_attr(attr)){
            auto &inArr = prim->polys.attr<float>(attr);
            auto factor = ZImpl(get_input2<float>("divisor"));
            if (attrOut == attr) {
                std::vector<int> outArr(inArr.size());
                parallel_for(inArr.size(), [&] (size_t i) {
                    outArr[i] = std::rint(inArr[i] * factor);
                });
                prim->polys.attrs.erase(attrOut);
                prim->polys.add_attr<int>(attrOut) = std::move(outArr);
            } else {
                auto &outArr = prim->polys.add_attr<int>(attrOut);
                parallel_for(inArr.size(), [&] (size_t i) {
                    outArr[i] = std::rint(inArr[i] * factor);
                });
            }
        }
        ZImpl(set_output("prim", std::move(prim)));
    }
};

ZENDEFNODE(PrimFloatAttrToInt, {
    {
    {gParamType_Primitive, "prim", "", zeno::Socket_ReadOnly},
    {gParamType_String, "attr", "tag"},
    {gParamType_String, "attrOut", "tag"},
    {gParamType_Float, "divisor", "1"},
    },
    {
    {gParamType_Primitive, "prim"},
    },
    {
    },
    {"primitive"},
});

struct PrimIntAttrToFloat : INode {
    virtual void apply() override {
        auto prim = ZImpl(get_input<PrimitiveObject>("prim"));
        auto attr = ZImpl(get_input2<std::string>("attr"));
        auto attrOut = ZImpl(get_input2<std::string>("attrOut"));


        if(prim->verts.has_attr(attr)){
        auto &inArr = prim->verts.attr<int>(attr);
        auto factor = ZImpl(get_input2<float>("divisor"));
        if (factor) factor = 1.0f / factor;
        if (attrOut == attr) {
            std::vector<float> outArr(inArr.size());
            parallel_for(inArr.size(), [&] (size_t i) {
                outArr[i] = float(inArr[i]) * factor;
            });
            prim->verts.attrs.erase(attrOut);
            prim->verts.add_attr<float>(attrOut) = std::move(outArr);
        } else {
            auto &outArr = prim->verts.add_attr<float>(attrOut);
            parallel_for(inArr.size(), [&] (size_t i) {
                outArr[i] = float(inArr[i]) * factor;
            });
        }
        }
        if(prim->tris.has_attr(attr)){
            auto &inArr = prim->tris.attr<int>(attr);
            auto factor = ZImpl(get_input2<float>("divisor"));
            if (factor) factor = 1.0f / factor;
            if (attrOut == attr) {
                std::vector<float> outArr(inArr.size());
                parallel_for(inArr.size(), [&] (size_t i) {
                    outArr[i] = float(inArr[i]) * factor;
                });
                prim->tris.attrs.erase(attrOut);
                prim->tris.add_attr<float>(attrOut) = std::move(outArr);
            } else {
                auto &outArr = prim->tris.add_attr<float>(attrOut);
                parallel_for(inArr.size(), [&] (size_t i) {
                    outArr[i] = float(inArr[i]) * factor;
                });
            }
        }
        if(prim->polys.has_attr(attr)){
            auto &inArr = prim->polys.attr<int>(attr);
            auto factor = ZImpl(get_input2<float>("divisor"));
            if (factor) factor = 1.0f / factor;
            if (attrOut == attr) {
                std::vector<float> outArr(inArr.size());
                parallel_for(inArr.size(), [&] (size_t i) {
                    outArr[i] = float(inArr[i]) * factor;
                });
                prim->polys.attrs.erase(attrOut);
                prim->polys.add_attr<float>(attrOut) = std::move(outArr);
            } else {
                auto &outArr = prim->polys.add_attr<float>(attrOut);
                parallel_for(inArr.size(), [&] (size_t i) {
                    outArr[i] = float(inArr[i]) * factor;
                });
            }
        }
        ZImpl(set_output("prim", std::move(prim)));
    }
};

ZENDEFNODE(PrimIntAttrToFloat, {
    {
    {gParamType_Primitive, "prim", "", zeno::Socket_ReadOnly},
    {gParamType_String, "attr", "tag"},
    {gParamType_String, "attrOut", "tag"},
    {gParamType_Float, "divisor", "1"},
    },
    {
    {gParamType_Primitive, "prim"},
    },
    {
    },
    {"primitive"},
});

struct PrimAttrInterp : INode {
    virtual void apply() override {
        auto prim = ZImpl(get_input<PrimitiveObject>("prim"));
        auto prim2 = ZImpl(get_input<PrimitiveObject>("prim2"));
        auto attr = ZImpl(get_input2<std::string>("attr"));
        auto factor = ZImpl(get_input2<float>("factor"));
        auto facAttr = ZImpl(get_input2<std::string>("facAttr"));
        auto facAcc = functor_variant(facAttr.empty() ? 1 : 0,
                                      [&, &facAttr = facAttr] {
                                          auto &facArr = prim->verts.attr<float>(facAttr);
                                          return [&] (size_t i) {
                                              return facArr[i];
                                          };
                                      },
                                      [&] {
                                          return [&] (size_t i) {
                                              return factor;
                                          };
                                      });
        auto process = [&] (std::string const &key, auto &arr) {
            using T = std::decay_t<decltype(arr[0])>;
            auto &arr2 = prim2->add_attr<T>(key);
            std::visit([&] (auto const &facAcc) {
                parallel_for(std::min(arr.size(), arr2.size()), [&] (size_t i) {
                    arr[i] = (T)mix(arr[i], arr2[i], facAcc(i));
                });
            }, facAcc);
        };
        if (attr.empty()) {
            prim->foreach_attr<AttrAcceptAll>([&] (auto const &key, auto &arr) {
                if (!facAttr.empty() && key == facAttr)
                    return;
                process(key, arr);
            });
        } else {
            prim->attr_visit<AttrAcceptAll>(attr, [&] (auto &arr) {
                process(attr, arr);
            });
        }
        ZImpl(set_output("prim", std::move(prim)));
    }
};

ZENDEFNODE(PrimAttrInterp, {
    {
    {gParamType_Primitive, "prim", "", zeno::Socket_ReadOnly},
    {gParamType_Primitive, "prim2", "", zeno::Socket_ReadOnly},
    {gParamType_String, "attr", ""},
    {gParamType_Float, "factor", "0.5"},
    {gParamType_String, "facAttr", ""},
    },
    {
    {gParamType_Primitive, "prim"},
    },
    {
    },
    {"primitive"},
});

template <class T>
static void prim_remap(std::vector<T> &arr, bool autocompute, float inMax, float inMin, float outputMax, float outputMin, bool clampMax, bool clampMin, bool ramp, const CurvesData *curve)
{
        if (autocompute) {
            inMin = zeno::parallel_reduce_array<T>(arr.size(), arr[0], [&] (size_t i) -> T { return arr[i]; },
            [&] (T i, T j) -> T { return zeno::min(i, j); });
            inMax = zeno::parallel_reduce_array<T>(arr.size(), arr[0], [&] (size_t i) -> T { return arr[i]; },
            [&] (T i, T j) -> T { return zeno::max(i, j); });
        }
        float val = 0.0;
        float denom = inMax - inMin;
        if(denom == 0.0) {
            parallel_for(arr.size(), [&] (size_t i) {
                val = (arr[i] < inMin ? 0. : 1.);
                if (ramp) val = curve->eval(val);
                arr[i] = val * (outputMax - outputMin) + outputMin;
            });
        }
        else if constexpr (std::is_same_v<T, float>) {
            parallel_for(arr.size(), [&] (size_t i) {
                if(clampMax) arr[i] = zeno::min(arr[i], inMax);
                if(clampMin) arr[i] = zeno::max(arr[i], inMin);
                arr[i] = (arr[i] - inMin) / (denom);
                if (ramp) arr[i] = curve->eval(arr[i]);
                arr[i] = arr[i] * (outputMax - outputMin) + outputMin;
            });
        } else if constexpr (std::is_same_v<T, int>) {
            parallel_for(arr.size(), [&] (size_t i) {
                if(clampMax) arr[i] = zeno::min(arr[i], inMax);
                if(clampMin) arr[i] = zeno::max(arr[i], inMin);
                val = (arr[i] - inMin) / (denom);
                if (ramp) val = curve->eval(val);
                arr[i] = (int) std::round(val * (outputMax - outputMin) + outputMin);
            });
        }
}


struct PrimAttrRemap : INode {
    virtual void apply() override {// change attr name to create new attr?
        auto prim = ZImpl(get_input<PrimitiveObject>("prim"));
        auto attr = ZImpl(get_input2<std::string>("attr"));
        auto scope = ZImpl(get_input2<std::string>("scope"));
        auto autoCompute = ZImpl(get_input2<bool>("Auto Compute input range"));
        auto inMin = ZImpl(get_input2<float>("Input min"));
        auto inMax = ZImpl(get_input2<float>("Input max"));
        auto outputMin = ZImpl(get_input2<float>("Output min"));
        auto outputMax = ZImpl(get_input2<float>("Output max"));
        auto clampMin = ZImpl(get_input2<bool>("Clamp min"));
        auto clampMax = ZImpl(get_input2<bool>("Clamp max"));
        auto curve = ZImpl(get_input_prim<CurvesData>("Remap Ramp"));
        auto ramp = ZImpl(get_input2<bool>("Use Ramp"));
        if (scope == "vert"){
            if (prim->verts.attr_is<float>(attr)){
                auto &arr = prim->verts.attr<float>(attr);
                prim_remap<float>(arr, autoCompute, inMax, inMin, outputMax, outputMin, clampMax, clampMin, ramp, &curve);
                }
            else if (prim->verts.attr_is<int>(attr)){
                auto &arr = prim->verts.attr<int>(attr);
                prim_remap<int>(arr, autoCompute, inMax, inMin, outputMax, outputMin, clampMax, clampMin, ramp, &curve);
                }
            else{
                throw std::runtime_error("PrimAttrRemap: loops attr type not supported");
            }
        }
        else if (scope == "tri"){
            if (prim->tris.attr_is<float>(attr)){
                auto &arr = prim->tris.attr<float>(attr);
                prim_remap<float>(arr, autoCompute, inMax, inMin, outputMax, outputMin, clampMax, clampMin, ramp, &curve);
                }
            else if (prim->tris.attr_is<int>(attr)){
                auto &arr = prim->tris.attr<int>(attr);
                prim_remap<int>(arr, autoCompute, inMax, inMin, outputMax, outputMin, clampMax, clampMin, ramp, &curve);
                }
            else{
                throw std::runtime_error("PrimAttrRemap: loops attr type not supported");
            }
        }
        else if (scope == "loop"){
            if (prim->loops.attr_is<float>(attr)){
                auto &arr = prim->loops.attr<float>(attr);
                prim_remap<float>(arr, autoCompute, inMax, inMin, outputMax, outputMin, clampMax, clampMin, ramp, &curve);
                }
            else if (prim->loops.attr_is<int>(attr)){
                auto &arr = prim->loops.attr<int>(attr);
                prim_remap<int>(arr, autoCompute, inMax, inMin, outputMax, outputMin, clampMax, clampMin, ramp, &curve);
                }
            else{
                throw std::runtime_error("PrimAttrRemap: loops attr type not supported");
            }
        }
        else if (scope == "poly"){
            if (prim->polys.attr_is<float>(attr)){
                auto &arr = prim->polys.attr<float>(attr);
                prim_remap<float>(arr, autoCompute, inMax, inMin, outputMax, outputMin, clampMax, clampMin, ramp, &curve);
                }
            else if (prim->polys.attr_is<int>(attr)){
                auto &arr = prim->polys.attr<int>(attr);
                prim_remap<int>(arr, autoCompute, inMax, inMin, outputMax, outputMin, clampMax, clampMin, ramp, &curve);
                }
            else{
                throw std::runtime_error("PrimAttrRemap: loops attr type not supported");
            }
        }
        else if (scope == "line"){
            if (prim->lines.attr_is<float>(attr)){
                auto &arr = prim->lines.attr<float>(attr);
                prim_remap<float>(arr, autoCompute, inMax, inMin, outputMax, outputMin, clampMax, clampMin, ramp, &curve);
                }
            else if (prim->lines.attr_is<int>(attr)){
                auto &arr = prim->lines.attr<int>(attr);
                prim_remap<int>(arr, autoCompute, inMax, inMin, outputMax, outputMin, clampMax, clampMin, ramp, &curve);
                }
            else{
                throw std::runtime_error("PrimAttrRemap: loops attr type not supported");
            }
        }

        ZImpl(set_output("prim", std::move(prim)));
    }
};

ZENDEFNODE(PrimAttrRemap, {
    {
        {gParamType_Primitive, "prim", "", zeno::Socket_ReadOnly},
        {"enum vert tri loop poly line", "scope", "vert"},
        {gParamType_String, "attr", ""},
        {gParamType_Bool, "Auto Compute input range", "0"},
        {gParamType_Bool, "Clamp min", "0"},
        {gParamType_Bool, "Clamp max", "0"},
        {gParamType_Float, "Input min", "0"},
        {gParamType_Float, "Input max", "1"},
        {gParamType_Float, "Output min", "0"},
        {gParamType_Float, "Output max", "1"},
        {gParamType_Bool, "Use Ramp", "0"},
        {gParamType_Curve, "Remap Ramp"},
        
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


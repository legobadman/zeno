#include <zeno/zeno.h>
#include <zeno/types/PrimitiveObject.h>
#include <zeno/types/NumericObject.h>
#include <zeno/utils/random.h>
#include <zeno/utils/vec.h>
#include <cstring>
#include <cstdlib>

namespace zeno {

struct PrimitiveFillAttr : INode {
  virtual void apply() override {
    auto prim = ZImpl(get_input<PrimitiveObject>("prim"));
    auto value = ZImpl(get_input<NumericObject>("value"))->value;
    auto attrName = ZImpl(get_param<std::string>(("attrName")));
    auto attrType = ZImpl(get_param<std::string>(("attrType")));
    if (std::holds_alternative<vec3f>(value)) {
        attrType = "float3";
    }
    if (!prim->has_attr(attrName)) {
        if (attrType == "float3") prim->add_attr<zeno::vec3f>(attrName);
        else if (attrType == "float") prim->add_attr<float>(attrName);
    }
    auto &arr = prim->attr(attrName);
    std::visit([](auto &arr, auto const &value) {
        if constexpr (is_vec_castable_v<decltype(arr[0]), decltype(value)>) {
            #pragma omp parallel for
            for (int i = 0; i < arr.size(); i++) {
                arr[i] = decltype(arr[i])(value);
            }
        } else {
            throw Exception((std::string)"Failed to promote variant type from " + typeid(value).name() + " to " + typeid(arr[0]).name());
        }
    }, arr, value);

    ZImpl(set_output("prim", ZImpl(clone_input("prim"))));
  }
};

ZENDEFNODE(PrimitiveFillAttr,{
    {
        {gParamType_Primitive,"prim", "", zeno::Socket_ReadOnly},
        {gParamType_Vec3f, "value"},
    },
    {
{gParamType_Primitive, "prim"},
},
    {
        {gParamType_String, "attrName", "pos"},
        {"enum float float3 none", "attrType", "none"},
    },
    {
    "deprecated",
    }
});


static void print_cout(float x) {
    printf("%f\n", x);
}

static void print_cout(vec3f const &a) {
    printf("%f %f %f\n", a[0], a[1], a[2]);
}


struct PrimitivePrintAttr : INode {
  virtual void apply() override {
    auto prim = ZImpl(get_input<PrimitiveObject>("prim"));
    auto attrName = ZImpl(get_param<std::string>(("attrName")));
    prim->attr_visit(attrName, [attrName](auto const &arr) {
        printf("attribute `%s`, length %zd:\n", attrName.c_str(), arr.size());
        for (int i = 0; i < arr.size(); i++) {
            print_cout(arr[i]);
        }
        if (arr.size() == 0) {
            printf("(no data)\n");
        }
        printf("\n");
    });

    ZImpl(set_output("prim", ZImpl(clone_input("prim"))));
  }
};

ZENDEFNODE(PrimitivePrintAttr,{
    {
        {gParamType_Primitive,"prim", "", zeno::Socket_ReadOnly},
    },
    {
{gParamType_Primitive, "prim"},
},
    {
        {gParamType_String, "attrName", "pos"},
    },
    {
        "deprecated",
    }
});


// deprecated: use PrimitiveRandomAttr instead
struct PrimitiveRandomizeAttr : INode {
  virtual void apply() override {
    auto prim = ZImpl(get_input<PrimitiveObject>("prim"));
    auto min = ZImpl(get_param<float>(("min")));
    auto minY = ZImpl(get_param<float>(("minY")));
    auto minZ = ZImpl(get_param<float>(("minZ")));
    auto max = ZImpl(get_param<float>(("max")));
    auto maxY = ZImpl(get_param<float>(("maxY")));
    auto maxZ = ZImpl(get_param<float>(("maxZ")));
    auto attrName = ZImpl(get_param<std::string>(("attrName")));
    auto attrType = ZImpl(get_param<std::string>(("attrType")));
    if (!prim->has_attr(attrName)) {
        if (attrType == "float3") prim->add_attr<zeno::vec3f>(attrName);
        else if (attrType == "float") prim->add_attr<float>(attrName);
    }
    prim->attr_visit(attrName, [min, minY, minZ, max, maxY, maxZ](auto &arr) {
        for (int i = 0; i < arr.size(); i++) {
            if constexpr (is_decay_same_v<decltype(arr[i]), vec3f>) {
                // note: can't parallelize cuz frand() uses drand48() or rand()
                vec3f f(frand(), frand(), frand());
                vec3f a(min, minY, minZ);
                vec3f b(max, maxY, maxZ);
                arr[i] = mix(a, b, f);
            } else {
                arr[i] = mix(min, max, (float)frand());
            }
        }
    });

    ZImpl(set_output("prim", ZImpl(clone_input("prim"))));
  }
};

ZENDEFNODE(PrimitiveRandomizeAttr, {
    {
        {gParamType_Primitive, "prim", "", zeno::Socket_ReadOnly},
    },
    {
{gParamType_Primitive, "prim"},
},
    {
        {gParamType_String, "attrName", "pos"},
        {"enum float float3", "attrType", "float3"},
        {gParamType_Float, "min", "-1"},
        {gParamType_Float, "minY", "-1"},
        {gParamType_Float, "minZ", "-1"},
        {gParamType_Float, "max", "1"},
        {gParamType_Float, "maxY", "1"},
        {gParamType_Float, "maxZ", "1"},
    },
    {
        "deprecated",
    }
});


struct PrimitiveRandomAttr : INode {
  virtual void apply() override {
    auto prim = ZImpl(has_input("prim")) ?
        ZImpl(get_input<PrimitiveObject>("prim")) :
        std::make_shared<PrimitiveObject>();
    auto min = ZImpl(get_input<NumericObject>("min"));
    auto max = ZImpl(get_input<NumericObject>("max"));
    auto attrName = ZImpl(get_param<std::string>(("attrName")));
    auto attrType = ZImpl(get_param<std::string>(("attrType")));
    if (!prim->has_attr(attrName)) {
        if (attrType == "float3") prim->add_attr<zeno::vec3f>(attrName);
        else if (attrType == "float") prim->add_attr<float>(attrName);
    }
    prim->attr_visit(attrName, [&](auto &arr) {
        for (int i = 0; i < arr.size(); i++) {
            if constexpr (is_decay_same_v<decltype(arr[i]), vec3f>) {
                vec3f f(frand(), frand(), frand());
                auto a = min->is<float>() ? (vec3f)min->get<float>() : min->get<zeno::vec3f>();
                auto b = max->is<float>() ? (vec3f)max->get<float>() : max->get<zeno::vec3f>();
                arr[i] = mix(a, b, f);
            } else {
                float f(frand());
                auto a = min->get<float>();
                auto b = max->get<float>();
                arr[i] = mix(a, b, f);
            }
        }
    });

    ZImpl(set_output("prim", ZImpl(clone_input("prim"))));
  }
};

ZENDEFNODE(PrimitiveRandomAttr, {
    {
        {gParamType_Primitive, "prim", "", zeno::Socket_ReadOnly},
        {gParamType_Float, "min", "-1"},
        {gParamType_Float, "max", "1"},
    },
    {
        {gParamType_Primitive, "prim"},
    },
    {
        {gParamType_String, "attrName", "pos"},
        {"enum float float3", "attrType", ""},
    },
    {
        "deprecated",
    }
});

}

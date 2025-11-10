#include <zeno/zeno.h>
#include <zeno/types/StringObject.h>
#include <zeno/types/PrimitiveObject.h>
#include <zeno/types/IGeometryObject.h>
#include <zeno/funcs/PrimitiveUtils.h>
#include <zeno/types/NumericObject.h>
#include <zeno/utils/wangsrng.h>
#include <zeno/utils/variantswitch.h>
#include <zeno/utils/arrayindex.h>
#include <zeno/utils/orthonormal.h>
#include <zeno/para/parallel_for.h>
#include <zeno/para/task_group.h>
#include <zeno/utils/overloaded.h>
#include <zeno/utils/vec.h>
#include <zeno/utils/log.h>
#include <cstring>
#include <cstdlib>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace zeno {



namespace {

struct PrimDuplicate : INode {
    virtual void apply() override {
        auto parsPrim = get_input_Geometry("parsPrim")->toPrimitiveObject();
        auto meshPrim = get_input_Geometry("meshPrim")->toPrimitiveObject();
        auto tanAttr = ZImpl(get_input2<std::string>("tanAttr"));
        auto dirAttr = ZImpl(get_input2<std::string>("dirAttr"));
        auto radAttr = ZImpl(get_input2<std::string>("radAttr"));
        auto onbType = ZImpl(get_input2<std::string>("onbType"));
        auto radius = ZImpl(get_input2<float>("radius"));
        auto copyParsAttr = ZImpl(get_input2<bool>("copyParsAttr"));
        auto copyMeshAttr = ZImpl(get_input2<bool>("copyMeshAttr"));
        auto prim = primDuplicate(parsPrim.get(), meshPrim.get(),
                                  dirAttr, tanAttr, radAttr, onbType,
                                  radius, copyParsAttr, copyMeshAttr);
        ZImpl(set_output("prim", create_GeometryObject(prim.get())));
    }
};

ZENDEFNODE(PrimDuplicate, {
    {
    {gParamType_Geometry, "parsPrim", "", zeno::Socket_ReadOnly},
    {gParamType_Geometry, "meshPrim", "", zeno::Socket_ReadOnly},
    {gParamType_String, "dirAttr", ""},
    {gParamType_String, "tanAttr", ""},
    {gParamType_String, "radAttr", ""},
    {"enum XYZ YXZ YZX ZYX ZXY XZY", "onbType", "XYZ"},
    {gParamType_Float, "radius", "1"},
    {gParamType_Bool, "copyParsAttr", "1"},
    {gParamType_Bool, "copyMeshAttr", "1"},
    },
    {
    {gParamType_Geometry, "prim"},
    },
    {
    },
    {"primitive"},
});

struct PrimDuplicateConnLines : INode {
    virtual void apply() override {
        auto profPrim = ZImpl(get_input<PrimitiveObject>("parsPrim"));
        auto prim = ZImpl(get_input<PrimitiveObject>("meshPrim"));
        auto tanAttr = ZImpl(get_input2<std::string>("tanAttr"));
        auto dirAttr = ZImpl(get_input2<std::string>("dirAttr"));
        auto radAttr = ZImpl(get_input2<std::string>("radAttr"));
        auto onbType = ZImpl(get_input2<std::string>("onbType"));
        auto radius = ZImpl(get_input2<float>("radius"));
        auto copyParsAttr = ZImpl(get_input2<bool>("copyParsAttr"));
        auto copyMeshAttr = ZImpl(get_input2<bool>("copyMeshAttr"));
        auto outprim = primDuplicate(profPrim.get(), prim.get(),
                                  dirAttr, tanAttr, radAttr, onbType,
                                  radius, copyParsAttr, copyMeshAttr);
        outprim->lines.clear();
        outprim->quads.reserve(prim->lines.size() * profPrim->lines.size());
        for (size_t i = 0; i < prim->lines.size(); i++) {
            for (size_t j = 0; j < profPrim->lines.size(); j++) {
                auto [k1, k2] = profPrim->lines[j];
                auto a = prim->lines[i] + k1 * prim->verts.size();
                auto b = prim->lines[i] + k2 * prim->verts.size();
                outprim->quads.emplace_back(a[0], a[1], b[1], b[0]);
            }
        }
        outprim->quads.update();
        ZImpl(set_output("prim", std::move(outprim)));
    }
};

ZENDEFNODE(PrimDuplicateConnLines, {
    {
    {gParamType_Primitive, "parsPrim", "", zeno::Socket_ReadOnly},
    {gParamType_Primitive, "meshPrim", "", zeno::Socket_ReadOnly},
    {gParamType_String, "dirAttr", ""},
    {gParamType_String, "tanAttr", ""},
    {gParamType_String, "radAttr", ""},
    {"enum XYZ YXZ YZX ZYX ZXY XZY", "onbType", "XYZ"},
    {gParamType_Float, "radius", "1"},
    {gParamType_Bool, "copyParsAttr", "1"},
    {gParamType_Bool, "copyMeshAttr", "1"},
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

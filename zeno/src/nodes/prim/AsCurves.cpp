#include <zeno/types/PrimitiveObject.h>
#include <zeno/types/CurveType.h>
#include <zeno/types/UserData.h>
#include <zeno/zeno.h>
#include <zeno/types/IGeometryObject.h>
#include "magic_enum.hpp"

namespace zeno {

struct AsCurves : zeno::INode {
    virtual void apply() override {
        
        auto prim = clone_input_Geometry("prim");

        auto typeString = ZImpl(get_input2<std::string>("type"));
        auto typeEnum = magic_enum::enum_cast<CurveType>(typeString).value_or(CurveType::LINEAR);
        auto typeIndex = (int)magic_enum::enum_index<CurveType>(typeEnum).value_or(0);

        prim->userData()->set_int("curve", typeIndex);
        set_output("prim", std::move(prim));
    }
};

ZENDEFNODE(AsCurves, 
{   {   
        {gParamType_Geometry, "prim"},
    },
    {
        {gParamType_Geometry, "prim"}
    }, //output
    {
        {"enum " + zeno::CurveTypeListString(), "type", zeno::CurveTypeDefaultString() }
    },            //prim
    {"prim"}
});

} // namespace
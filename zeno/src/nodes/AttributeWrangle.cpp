#include <zeno/zeno.h>
#include <zeno/types/PrimitiveObject.h>
#include <zeno/types/IGeometryObject.h>
#include <zeno/formula/zfxexecute.h>
#include <zeno/core/FunctionManager.h>


namespace zeno
{
    struct AttributeWrangle : zeno::INode
    {
        void apply() override {
            auto spGeo = ZImpl(get_input2<GeometryObject_Adapter>("Input"));
            std::string zfxCode = ZImpl(get_input2<std::string>("Zfx Code"));
            std::string runOver = ZImpl(get_input2<std::string>("Run Over"));
            if (!spGeo) {
                throw makeError<UnimplError>("no geo");
            }

            ZfxContext ctx;
            ctx.spNode = this->m_pAdapter;
            ctx.spObject = spGeo;
            ctx.code = zfxCode;

            if (runOver == "Points") ctx.runover = ATTR_POINT;
            else if (runOver == "Faces") ctx.runover = ATTR_FACE;
            else if (runOver == "Geometry") ctx.runover = ATTR_GEO;

            ZfxExecute zfx(zfxCode, &ctx);
            zfx.execute();

            ZImpl(set_output("Output", spGeo));
        }
    };

    ZENDEFNODE(AttributeWrangle,
    {
        {
            {gParamType_Geometry, "Input"},
            ParamPrimitive("Run Over", gParamType_String, "Points", Combobox, std::vector<std::string>{"Points", "Faces", "Geometry"}),
            ParamPrimitive("Zfx Code", gParamType_String, "", CodeEditor)
        },
        {
            {gParamType_Geometry, "Output"}
        },
        {},
        {"geom"}
    });
}
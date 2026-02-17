#include <zeno/zeno.h>
#include <zeno/types/PrimitiveObject.h>
#include <zeno/types/GeometryObject.h>
#include <zeno/formula/zfxexecute.h>
#include <zeno/core/FunctionManager.h>


namespace zeno
{
    struct AttributeWrangle : zeno::INode2
    {
        NodeType type() const { return Node_Normal; }
        void clearCalcResults() {}
        float time() const { return 1.0f; }

        ZErrorCode apply(INodeData* pParams) override {
            auto params = static_cast<ZNodeParams*>(pParams);
            auto spGeo = params->get_input_Geometry("Input");
            std::string zfxCode = params->get_input2_string("Zfx Code");
            std::string runOver = params->get_input2_string("Run Over");
            if (!spGeo) {
                throw makeError<UnimplError>("no geo");
            }

            ZfxContext ctx;
            ctx.spNode = params->getNode();
            ctx.spObject.reset(spGeo->clone());
            ctx.code = zfxCode;

            if (runOver == "Points") ctx.runover = ATTR_POINT;
            else if (runOver == "Faces") ctx.runover = ATTR_FACE;
            else if (runOver == "Geometry") ctx.runover = ATTR_GEO;

            ZfxExecute zfx(zfxCode, &ctx);
            zfx.execute();

            params->set_output("Output", std::move(ctx.spObject));
            return ZErr_OK;
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
        {"geom"},
        "execute zfx to modify geometry",
        "aw",
        ":/icons/node/aw.svg",
        "#DE6E10"
    });
}
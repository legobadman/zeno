#include <zeno/zeno.h>
#include <zeno/types/PrimitiveObject.h>
#include <zeno/types/IGeometryObject.h>
#include <zeno/formula/zfxexecute.h>
#include <zeno/core/FunctionManager.h>


namespace zeno
{
    struct AttributeWrangle : zeno::INode
    {
        CustomUI export_customui() const override {
            CustomUI ui = INode::export_customui();
            ui.uistyle.iconResPath = "<svg width=\"26\" height=\"26\" viewBox=\"0 0 26 26\" fill=\"none\" xmlns=\"http://www.w3.org/2000/svg\"><path fill-rule=\"evenodd\" clip-rule=\"evenodd\" d=\"M7.22585 25.0714H1.24739L7.85951 11.7446L10.8555 17.6921L7.22585 25.0714ZM8.29095 10.471C8.20987 10.3109 8.04096 10.2092 7.85661 10.2092H7.85516C7.66936 10.2096 7.50045 10.3122 7.42082 10.4734L0.0471582 25.3356C-0.0242668 25.4795 -0.0141321 25.6485 0.0746665 25.7836C0.1625 25.9183 0.316932 26 0.482464 26H7.53134C7.7181 26 7.8875 25.8965 7.96761 25.7344L11.827 17.888C11.8892 17.7608 11.8887 17.6136 11.8246 17.4873L8.29095 10.471ZM25.0348 25.0714H19.0949L9.5655 5.83607C9.48587 5.6745 9.316 5.57143 9.12971 5.57143H5.24332L5.24767 0.928571H12.865L22.3506 20.1635C22.4302 20.325 22.6001 20.4286 22.7864 20.4286H25.0348V25.0714ZM25.5174 19.5H23.0918L13.6063 0.265107C13.5267 0.103536 13.3568 0 13.17 0H4.76555C4.49964 0 4.28343 0.207536 4.28295 0.463821L4.27764 6.03525C4.27764 6.15875 4.32783 6.27668 4.41904 6.36396C4.50929 6.45125 4.63187 6.5 4.76024 6.5H8.82519L18.3551 25.7354C18.4352 25.8969 18.6041 26 18.7904 26H25.5174C25.7843 26 26 25.792 26 25.5357V19.9643C26 19.708 25.7843 19.5 25.5174 19.5Z\" fill=\"white\"/></svg>";
            ui.uistyle.background = "#DE6E10";
            return ui;
        }

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

            ZImpl(set_output("Output", ctx.spObject));
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
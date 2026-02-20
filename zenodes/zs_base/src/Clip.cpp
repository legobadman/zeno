#include <vec.h>
#include <inodedata.h>
#include <inodeimpl.h>
#include "api.h"
#include "zcommon.h"
#include "vecutil.h"
#include "typehelper.h"
#include "utils.h"
#include <Windows.h>
#include <vector>

#if 0
// Original implementation depends on divideObject from zeno/geo/geometryutil.h
// which is not exposed in the zenodes API.
/*
struct Clip : INode {
    void apply() override {
        auto input_object = ZImpl(get_input2<GeometryObject_Adapter>("Input Object"));
        std::string Keep = ZImpl(get_input2<std::string>("Keep"));
        zeno::vec3f center_pos = ZImpl(get_input2<zeno::vec3f>("Center Position"));
        zeno::vec3f direction = ZImpl(get_input2<zeno::vec3f>("Direction"));
        DivideKeep keep = ...;
        auto spOutput = divideObject(input_object.get(), keep, center_pos, direction);
        ZImpl(set_output("Output", std::move(spOutput)));
    }
};
*/
#endif

namespace zeno {

    struct Clip : INode2 {

        zeno::NodeType type() const override { return zeno::Node_Normal; }
        float time() const override { return 1.f; }
        void clearCalcResults() override {}

        ZErrorCode apply(zeno::INodeData* ptrNodeData) override
        {
#if 0
            auto input_geom = ptrNodeData->get_input_Geometry("Input Object");
            if (!input_geom) {
                ptrNodeData->report_error("empty input object");
                return ZErr_ParamError;
            }
            std::string Keep = get_string_param(ptrNodeData, "Keep");
            zeno::vec3f center_pos = toVec3f(ptrNodeData->get_input2_vec3f("Center Position"));
            zeno::vec3f direction = toVec3f(ptrNodeData->get_input2_vec3f("Direction"));
            // divideObject not in api
            return ZErr_OK;
#else
            ptrNodeData->report_error("Clip: requires divideObject API from zenocore geometryutil");
            return ZErr_UnimplError;
#endif
        }
    };

    ZENDEFNODE_ABI(Clip,
        Z_INPUTS(
            { "Input Object", _gParamType_Geometry },
            { "Keep", _gParamType_String, ZString("All"), Combobox, Z_STRING_ARRAY("All", "Part Below The Plane", "Part Above The Plane") },
            { "Center Position", _gParamType_Vec3f, ZVec3f(0,0,0) },
            { "Direction", _gParamType_Vec3f, ZVec3f(0,1,0) }
        ),
        Z_OUTPUTS(
            { "Output", _gParamType_Geometry }
        ),
        "prim",
        "",
        "",
        ""
    );
}

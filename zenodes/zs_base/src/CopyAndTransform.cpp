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

namespace zeno {

    struct CopyAndTransform : INode2 {

        zeno::NodeType type() const override { return zeno::Node_Normal; }
        float time() const override { return 1.f; }
        void clearCalcResults() override {}

        ZErrorCode apply(zeno::INodeData* ptrNodeData) override
        {
            ptrNodeData->report_error("CopyAndTransform: TODO");
            return ZErr_UnimplError;
        }
    };

    ZENDEFNODE_ABI(CopyAndTransform,
        Z_INPUTS(
            { "Input", _gParamType_Geometry },
            { "Total Number", _gParamType_Int, ZInt(2), Slider, Z_ARRAY(0, 20, 1) },
            { "Translate", _gParamType_Vec3f, ZVec3f(0,0,0) },
            { "Rotate", _gParamType_Vec3f, ZVec3f(0,0,0) },
            { "Scale", _gParamType_Vec3f, ZVec3f(1,1,1) },
            { "Uniform Scale", _gParamType_Float, ZFloat(1.0f) }
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

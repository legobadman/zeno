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
// Original implementation is large and uses getLineNextPt, complex sweep logic.
// Kept for reference when implementing.
#endif

namespace zeno {

    struct Sweep : INode2 {

        zeno::NodeType type() const override { return zeno::Node_Normal; }
        float time() const override { return 1.f; }
        void clearCalcResults() override {}

        ZErrorCode apply(zeno::INodeData* ptrNodeData) override
        {
            ptrNodeData->report_error("Sweep: not yet implemented");
            return ZErr_UnimplError;
        }
    };

    ZENDEFNODE_ABI(Sweep,
        Z_INPUTS(
            { "Input", _gParamType_Geometry },
            { "Snap Distance", _gParamType_Float, ZFloat(0.2f) },
            { "Surface Shape", _gParamType_String, ZString("Square Tube"), Combobox, Z_STRING_ARRAY("Second Input", "Square Tube") },
            { "Width", _gParamType_Float, ZFloat(0.2f) },
            { "Columns", _gParamType_Int, ZInt(2) }
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

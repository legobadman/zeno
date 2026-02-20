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

    struct Merge : INode2 {

        zeno::NodeType type() const override { return zeno::Node_Normal; }
        float time() const override { return 1.f; }
        void clearCalcResults() override {}

        ZErrorCode apply(zeno::INodeData* ptrNodeData) override
        {
            auto list_object = ptrNodeData->get_input_ListObject("Input Of Objects");
            if (!list_object) {
                ptrNodeData->report_error("empty input list");
                return ZErr_ParamError;
            }

            auto mergedObj = zeno::mergeObjects(list_object);
            ptrNodeData->set_output_object("Output", mergedObj);
            return ZErr_OK;
        }
    };

    ZENDEFNODE_ABI(Merge,
        Z_INPUTS(
            { "Input Of Objects", _gParamType_List }
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

#include <inodedata.h>
#include <inodeimpl.h>
#include "zcommon.h"
#include <Windows.h>
#include <string>
#include <vector>
#include <zenum.h>

namespace zeno {
    struct CustomNode2 : INode2 {
        zeno::NodeType type() const override {
            return zeno::Node_Normal;
        }
        void clearCalcResults() override {}
        ZErrorCode apply(INodeData* ptrNodeData) override {
            return ZErr_UnimplError;
        }
        void getIconResource(char* recv, size_t cap) override {}
        void getBackgroundClr(char* recv, size_t cap) override {}
        float time() const override { return 1.0; }
    };

    ZENDEFNODE_ABI(CustomNode2,
        Z_INPUTS(
            { "param1", _gParamType_Float, ZFloat(1.0f) },
            { "param2", _gParamType_Float, ZFloat(0.2f), Slider, Z_ARRAY(0, 1, 0.1) },
        ),
        Z_OUTPUTS(
            { "Output", _gParamType_Geometry }
        ),
        "debug",
        "custom node"
    );
}

#include "simple_geometry_common.h"

namespace zeno {

struct Route : INode2 {
    NodeType type() const override { return Node_Normal; }
    float time() const override { return 1.0f; }
    void clearCalcResults() override {}

    ZErrorCode apply(INodeData* nd) override {
        if (nd->has_input("input")) {
            nd->set_output_object("output", nd->clone_input_object("input"));
        }
        return ZErr_OK;
    }
};

ZENDEFNODE_ABI(Route,
    Z_INPUTS(
        {"input", _gParamType_IObject},
    ),
    Z_OUTPUTS(
        {"output", _gParamType_IObject},
    ),
    "layout",
    "",
    "",
    ""
);

} // namespace zeno


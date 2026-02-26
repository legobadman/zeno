#include "simple_geometry_common.h"

namespace zeno {

struct Point : INode2 {
    zeno::NodeType type() const override { return zeno::Node_Normal; }
    float time() const override { return 1.0f; }
    void clearCalcResults() override {}
    ZErrorCode apply(INodeData* ptrNodeData) override
    {
        zeno::Vec3f posAbi = ptrNodeData->get_input2_vec3f("Position");
        std::vector<glm::vec3> pos = { glm::vec3(posAbi.x, posAbi.y, posAbi.z) };
        auto geo = create_GeometryObject(Topo_HalfEdge, false, pos, {});
        ptrNodeData->set_output_object("Output", geo);
        return ZErr_OK;
    }
};

ZENDEFNODE_ABI(Point,
    Z_INPUTS(
        { "Position", _gParamType_Vec3f, ZVec3f(0,0,0) }
    ),
    Z_OUTPUTS(
        { "Output", _gParamType_Geometry }
    ),
    "create",
    "",
    ":/icons/node/point.svg",
    ""
);

} // namespace zeno


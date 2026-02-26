#include "simple_geometry_common.h"

namespace zeno {

struct Line : INode2 {
    zeno::NodeType type() const override { return zeno::Node_Normal; }
    float time() const override { return 1.0f; }
    void clearCalcResults() override {}
    ZErrorCode apply(INodeData* ptrNodeData) override
    {
        int npoints = ptrNodeData->get_input2_int("npoints");
        zeno::vec3f direction = toVec3f(ptrNodeData->get_input2_vec3f("direction"));
        zeno::vec3f origin = toVec3f(ptrNodeData->get_input2_vec3f("origin"));
        float length = ptrNodeData->get_input2_float("length");
        bool isCentered = ptrNodeData->get_input2_bool("isCentered");

        if (npoints < 2) {
            return ZErr_ParamError;
        }

        if (length < 0) {
            return ZErr_ParamError;
        }

        if (direction == zeno::vec3f({ 0,0,0 }))
            return ZErr_ParamError;

        float scale = length /
            glm::sqrt(glm::pow(direction[0], 2) +
                glm::pow(direction[1], 2) +
                glm::pow(direction[2], 2)) /
            (npoints - 1);

        zeno::vec3f ax = direction * scale;
        if (isCentered) {
            origin -= (ax * (npoints - 1)) / 2.0f;
        }

        std::vector<zeno::vec3f> points;
        std::vector<std::vector<int>> faces;

        points.resize(npoints);
        faces.resize(npoints - 1);

        for (int pt = 0; pt < npoints; ++pt) {
            zeno::vec3f p = origin + pt * ax;
            points[pt] = p;

            if (pt > 0) {
                faces[pt - 1] = { pt - 1, pt };
            }
        }

        auto geo = create_GeometryObject(Topo_HalfEdge, false, to_glm_points(points), faces);
        ptrNodeData->set_output_object("Output", geo);
        return ZErr_OK;
    }
};

ZENDEFNODE_ABI(Line,
    Z_INPUTS(
        { "direction", _gParamType_Vec3f, ZVec3f(0,1,0) },
        { "origin",    _gParamType_Vec3f, ZVec3f(0,0,0) },
        { "npoints",   _gParamType_Int,   ZInt(2) },
        { "length",    _gParamType_Float, ZFloat(1.0f) },
        { "isCentered",_gParamType_Bool,  ZInt(0) }
    ),
    Z_OUTPUTS(
        { "Output", _gParamType_Geometry }
    ),
    "create",
    "",
    ":/icons/node/line.svg",
    ""
);

} // namespace zeno


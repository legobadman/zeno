#include "simple_geometry_common.h"

namespace zeno {

struct Circle : INode2 {
    enum PlaneDirection {
        Dir_XY,
        Dir_YZ,
        Dir_ZX
    };

    zeno::NodeType type() const override { return zeno::Node_Normal; }
    float time() const override { return 1.0f; }
    void clearCalcResults() override {}
    ZErrorCode apply(INodeData* ptrNodeData) override
    {
        zeno::vec3f Center = toVec3f(ptrNodeData->get_input2_vec3f("Center"));
        zeno::vec3f Rotate = toVec3f(ptrNodeData->get_input2_vec3f("Rotate"));
        int segments = ptrNodeData->get_input2_int("Segments");
        float radius = ptrNodeData->get_input2_float("Radius");
        std::string direction = get_string_param(ptrNodeData, "Orientation");

        PlaneDirection dir;
        Rotate_Orientaion ori;

        if (direction == "ZX") {
            dir = Dir_ZX; ori = Orientaion_ZX;
        }
        else if (direction == "YZ") {
            dir = Dir_YZ; ori = Orientaion_YZ;
        }
        else {
            dir = Dir_XY; ori = Orientaion_XY;
        }

        if (segments < 3 || radius <= 0.0f) return ZErr_ParamError;

        std::vector<zeno::vec3f> points((size_t)segments);
        std::vector<std::vector<int>> faces;
        faces.reserve((size_t)segments);

        for (int i = 0; i < segments; ++i) {
            float rad = 2.0f * (float)M_PI * (float)i / (float)segments;
            zeno::vec3f pt;
            if (dir == Dir_ZX) {
                pt = { cos(rad) * radius, 0, -sin(rad) * radius };
            }
            else if (dir == Dir_YZ) {
                pt = { 0, cos(rad) * radius, -sin(rad) * radius };
            }
            else {
                pt = { cos(rad) * radius, -sin(rad) * radius, 0 };
            }
            points[(size_t)i] = pt;
        }

        for (int i = 0; i < segments; ++i) {
            int j = (i + 1) % segments;
            faces.push_back({ i, j });
        }

        glm::mat4 translate = glm::translate(glm::mat4(1.0f),
            glm::vec3(Center[0], Center[1], Center[2]));
        glm::mat4 rotation = calc_rotate_matrix(Rotate[0], Rotate[1], Rotate[2], ori);
        glm::mat4 transform = translate * rotation;

        for (size_t i = 0; i < points.size(); ++i) {
            auto& pt = points[i];
            glm::vec4 gp = transform * glm::vec4(pt[0], pt[1], pt[2], 1);
            pt = { gp.x,gp.y,gp.z };
        }

        auto geo = create_GeometryObject(Topo_HalfEdge, false, to_glm_points(points), faces);
        ptrNodeData->set_output_object("Output", geo);
        return ZErr_OK;
    }
};

ZENDEFNODE_ABI(Circle,
    Z_INPUTS(
        { "Center", _gParamType_Vec3f, ZVec3f(0,0,0) },
        { "Rotate", _gParamType_Vec3f, ZVec3f(0,0,0) },
        { "Segments", _gParamType_Int, ZInt(32), Lineedit },
        { "Radius", _gParamType_Float, ZFloat(1.0f), Lineedit },
        { "Orientation", _gParamType_String, ZString("ZX"), Combobox, Z_STRING_ARRAY("XY","YZ","ZX") }
    ),
    Z_OUTPUTS(
        { "Output", _gParamType_Geometry }
    ),
    "create",
    "",
    ":/icons/node/circle.svg",
    ""
);

} // namespace zeno


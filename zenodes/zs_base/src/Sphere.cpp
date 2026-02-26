#include "simple_geometry_common.h"

namespace zeno {

struct Sphere : INode2 {
    enum AxisDirection {
        Y_Axis,
        X_Axis,
        Z_Axis
    };

    zeno::NodeType type() const override { return zeno::Node_Normal; }
    float time() const override { return 1.0f; }
    void clearCalcResults() override {}

    ZErrorCode apply(INodeData* ptrNodeData) override
    {
        zeno::vec3f Center = toVec3f(ptrNodeData->get_input2_vec3f("Center"));
        zeno::vec3f Rotate = toVec3f(ptrNodeData->get_input2_vec3f("Rotate"));
        zeno::vec3f Radius = toVec3f(ptrNodeData->get_input2_vec3f("Radius"));
        float uniform_scale = ptrNodeData->get_input2_float("Uniform Scale");
        std::string Direction = get_string_param(ptrNodeData, "Direction");
        int Rows = ptrNodeData->get_input2_int("Rows");
        int Columns = ptrNodeData->get_input2_int("Columns");
        std::string face_type = get_string_param(ptrNodeData, "Face Type");

        bool bQuad = face_type == "Quadrilaterals";

        if (Rows < 3) return ZErr_ParamError;
        if (Columns < 2) return ZErr_ParamError;

        int nPoints = 2 + (Rows - 2) * Columns;
        int nFaces = bQuad ? (Rows - 1) * Columns
            : Columns * 2 + (Rows - 3) * Columns * 2;

        float Rx = Radius[0], Ry = Radius[1], Rz = Radius[2];
        if (Rx <= 0 || Ry <= 0 || Rz <= 0)
            return ZErr_ParamError;

        AxisDirection dir;
        Rotate_Orientaion ori;

        if (Direction == "Y Axis") {
            dir = Y_Axis; ori = Orientaion_ZX;
        }
        else if (Direction == "X Axis") {
            dir = X_Axis; ori = Orientaion_YZ;
        }
        else {
            dir = Z_Axis; ori = Orientaion_XY;
        }

        std::vector<zeno::vec3f> points(nPoints);
        std::vector<std::vector<int>> faces;
        faces.reserve(nFaces);

        zeno::vec3f topPos, bottomPos;
        if (dir == Y_Axis) { topPos = { 0,1,0 }; bottomPos = { 0,-1,0 }; }
        else if (dir == X_Axis) { topPos = { 1,0,0 }; bottomPos = { -1,0,0 }; }
        else { topPos = { 0,0,1 }; bottomPos = { 0,0,-1 }; }

        points[0] = topPos;
        points[1] = bottomPos;

        float x = 0, y = 0, z = 0;

        for (int row = 1; row < Rows - 1; row++)
        {
            float v = (float)row / (float)(Rows - 1);
            float theta = M_PI * v;

            if (dir == Y_Axis) y = cos(theta);
            else if (dir == X_Axis) x = cos(theta);
            else z = cos(theta);

            int startIdx = 2 + (row - 1) * Columns;

            for (int col = 0; col < Columns; col++)
            {
                float u = (float)col / (float)Columns;
                float phi = M_PI * 2 * u;

                if (dir == Y_Axis) {
                    x = sin(theta) * cos(phi);
                    z = -sin(theta) * sin(phi);
                }
                else if (dir == Z_Axis) {
                    x = sin(theta) * cos(phi);
                    y = sin(theta) * sin(phi);
                }
                else {
                    y = sin(theta) * cos(phi);
                    z = -sin(theta) * sin(phi);
                }

                int idx = 2 + (row - 1) * Columns + col;
                points[idx] = zeno::vec3f(x, y, z);

                if (col > 0) {
                    if (row == 1) {
                        faces.push_back({ 0, idx - 1, idx });
                        if (col == Columns - 1)
                            faces.push_back({ 0, idx, startIdx });
                    }
                    else {
                        int rd = idx;
                        int ld = rd - 1;
                        int ru = 2 + (row - 2) * Columns + col;
                        int lu = ru - 1;

                        if (bQuad)
                            faces.push_back({ rd,ru,lu,ld });
                        else {
                            faces.push_back({ rd,ru,lu });
                            faces.push_back({ rd,lu,ld });
                        }

                        if (col == Columns - 1) {
                            lu = ru;
                            ld = rd;
                            rd = startIdx;
                            ru = 2 + (row - 2) * Columns;

                            if (bQuad)
                                faces.push_back({ rd,ru,lu,ld });
                            else {
                                faces.push_back({ rd,ru,lu });
                                faces.push_back({ rd,lu,ld });
                            }
                        }

                        if (row == Rows - 2) {
                            faces.push_back({ idx, idx - 1, 1 });
                            if (col == Columns - 1)
                                faces.push_back({ 1, startIdx, idx });
                        }
                    }
                }
            }
        }

        glm::mat4 scale_matrix = glm::scale(glm::mat4(1.0),
            glm::vec3(uniform_scale * Rx, uniform_scale * Ry, uniform_scale * Rz));

        glm::mat4 translate = glm::translate(glm::mat4(1.0),
            glm::vec3(Center[0], Center[1], Center[2]));

        glm::mat4 rotation = calc_rotate_matrix(Rotate[0], Rotate[1], Rotate[2], ori);
        glm::mat4 transform = translate * rotation * scale_matrix;

        for (auto& p : points) {
            glm::vec4 gp = transform * glm::vec4(p[0], p[1], p[2], 1);
            p = zeno::vec3f(gp.x, gp.y, gp.z);
        }

        auto geo = create_GeometryObject(Topo_HalfEdge, !bQuad, to_glm_points(points), faces);
        ptrNodeData->set_output_object("Output", geo);
        return ZErr_OK;
    }
};

ZENDEFNODE_ABI(Sphere,
    Z_INPUTS(
        { "Center", _gParamType_Vec3f, ZVec3f(0,0,0) },
        { "Rotate", _gParamType_Vec3f, ZVec3f(0,0,0) },
        { "Radius", _gParamType_Vec3f, ZVec3f(1,1,1) },
        { "Uniform Scale", _gParamType_Float, ZFloat(1.0f) },
        { "Direction", _gParamType_String, ZString("Y Axis"), Combobox, Z_STRING_ARRAY("X Axis","Y Axis","Z Axis") },
        { "Rows", _gParamType_Int, ZInt(13), Slider, Z_ARRAY(3,100,1) },
        { "Columns", _gParamType_Int, ZInt(24), Slider, Z_ARRAY(2,100,1) },
        { "Face Type", _gParamType_String, ZString("Quadrilaterals"), Combobox, Z_STRING_ARRAY("Triangles","Quadrilaterals") }
    ),
    Z_OUTPUTS(
        { "Output", _gParamType_Geometry }
    ),
    "create",
    "",
    ":/icons/node/sphere.svg",
    ""
);

} // namespace zeno


#include <vec.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <inodedata.h>
#include <inodeimpl.h>
#include <Windows.h>
#include "api.h"
#include "zcommon.h"
#include "vecutil.h"
#include "typehelper.h"
#include "utils.h"
#include <vector>

namespace zeno {

    static std::vector<glm::vec3> to_glm_points_tube(const std::vector<zeno::vec3f>& v) {
        std::vector<glm::vec3> r(v.size());
        for (size_t i = 0; i < v.size(); i++) {
            r[i] = glm::vec3(v[i][0], v[i][1], v[i][2]);
        }
        return r;
    }

    static glm::mat4 euler_xyz_deg(float xangle, float yangle, float zangle) {
        constexpr float kDeg2Rad = 3.14159265358979323846f / 180.0f;
        const float rad_x = xangle * kDeg2Rad;
        const float rad_y = yangle * kDeg2Rad;
        const float rad_z = zangle * kDeg2Rad;

        glm::mat4 mx = glm::mat4(
            1, 0, 0, 0,
            0, cos(rad_x), sin(rad_x), 0,
            0, -sin(rad_x), cos(rad_x), 0,
            0, 0, 0, 1);
        glm::mat4 my = glm::mat4(
            cos(rad_y), 0, -sin(rad_y), 0,
            0, 1, 0, 0,
            sin(rad_y), 0, cos(rad_y), 0,
            0, 0, 0, 1);
        glm::mat4 mz = glm::mat4(
            cos(rad_z), sin(rad_z), 0, 0,
            -sin(rad_z), cos(rad_z), 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1);

        return mz * my * mx;
    }

    struct Tube : INode2 {
        zeno::NodeType type() const override { return zeno::Node_Normal; }
        float time() const override { return 1.0f; }
        void clearCalcResults() override {}

        ZErrorCode apply(INodeData* ptrNodeData) override {
            zeno::vec3f center = toVec3f(ptrNodeData->get_input2_vec3f("Center"));
            zeno::vec3f rotate = toVec3f(ptrNodeData->get_input2_vec3f("Rotate"));
            zeno::vec2f radius = toVec2f(ptrNodeData->get_input2_vec2f("Radius"));
            float radiusScale = ptrNodeData->get_input2_float("Radius Scale");
            float height = ptrNodeData->get_input2_float("Height");
            int rows = ptrNodeData->get_input2_int("Rows");
            int columns = ptrNodeData->get_input2_int("Columns");
            bool endCaps = ptrNodeData->get_input2_bool("End Caps");
            std::string orientation = get_string_param(ptrNodeData, "Orientation");

            if (rows < 2 || columns < 3) {
                return ZErr_ParamError;
            }
            if (height < 0.0f || radiusScale < 0.0f) {
                return ZErr_ParamError;
            }

            float topRadius = radius[0] * radiusScale;
            float bottomRadius = radius[1] * radiusScale;
            if (topRadius < 0.0f || bottomRadius < 0.0f) {
                return ZErr_ParamError;
            }

            std::vector<zeno::vec3f> points;
            std::vector<std::vector<int>> faces;
            points.reserve((size_t)rows * (size_t)columns);

            constexpr float kPi = 3.14159265358979323846f;
            float halfHeight = 0.5f * height;

            for (int r = 0; r < rows; ++r) {
                float t = (rows == 1) ? 0.0f : (float)r / (float)(rows - 1);
                float ringRadius = bottomRadius + (topRadius - bottomRadius) * t;
                float y = -halfHeight + height * t;

                for (int c = 0; c < columns; ++c) {
                    float u = (float)c / (float)columns;
                    float ang = 2.0f * kPi * u;
                    float x = ringRadius * cos(ang);
                    float z = -ringRadius * sin(ang);
                    points.emplace_back(x, y, z);
                }
            }

            // Side faces.
            for (int r = 0; r < rows - 1; ++r) {
                int row0 = r * columns;
                int row1 = (r + 1) * columns;
                for (int c = 0; c < columns; ++c) {
                    int c1 = (c + 1) % columns;
                    int a = row0 + c;
                    int b = row0 + c1;
                    int e = row1 + c1;
                    int d = row1 + c;
                    faces.push_back({ a, b, e, d });
                }
            }

            // End caps as n-gons.
            if (endCaps) {
                constexpr float kEps = 1e-8f;
                if (bottomRadius > kEps) {
                    std::vector<int> bottomFace;
                    bottomFace.reserve(columns);
                    for (int c = 0; c < columns; ++c) {
                        bottomFace.push_back(c);
                    }
                    faces.push_back(std::move(bottomFace));
                }
                if (topRadius > kEps) {
                    std::vector<int> topFace;
                    topFace.reserve(columns);
                    int topStart = (rows - 1) * columns;
                    for (int c = columns - 1; c >= 0; --c) {
                        topFace.push_back(topStart + c);
                    }
                    faces.push_back(std::move(topFace));
                }
            }

            glm::mat4 orient = glm::mat4(1.0f);
            if (orientation == "X Axis" || orientation == "X") {
                orient = glm::rotate(glm::mat4(1.0f), -0.5f * kPi, glm::vec3(0, 0, 1));
            }
            else if (orientation == "Z Axis" || orientation == "Z") {
                orient = glm::rotate(glm::mat4(1.0f), 0.5f * kPi, glm::vec3(1, 0, 0));
            }
            // Default is Y axis.

            glm::mat4 userRot = euler_xyz_deg(rotate[0], rotate[1], rotate[2]);
            glm::mat4 translate = glm::translate(glm::mat4(1.0f), glm::vec3(center[0], center[1], center[2]));
            glm::mat4 transform = translate * userRot * orient;

            for (auto& p : points) {
                glm::vec4 gp = transform * glm::vec4(p[0], p[1], p[2], 1.0f);
                p = zeno::vec3f(gp.x, gp.y, gp.z);
            }

            auto geo = create_GeometryObject(Topo_HalfEdge, false, to_glm_points_tube(points), faces);
            ptrNodeData->set_output_object("Output", geo);
            return ZErr_OK;
        }
    };

    ZENDEFNODE_ABI(Tube,
        Z_INPUTS(
            { "Orientation", _gParamType_String, ZString("Y Axis"), Combobox, Z_STRING_ARRAY("X Axis", "Y Axis", "Z Axis") },
            { "End Caps", _gParamType_Bool, ZInt(1), Checkbox },
            { "Center", _gParamType_Vec3f, ZVec3f(0,0,0) },
            { "Rotate", _gParamType_Vec3f, ZVec3f(0,0,0) },
            { "Radius", _gParamType_Vec2f, ZVec2f(0.5f,0.5f) },
            { "Radius Scale", _gParamType_Float, ZFloat(1.0f), Lineedit },
            { "Height", _gParamType_Float, ZFloat(1.0f), Lineedit },
            { "Rows", _gParamType_Int, ZInt(2), Slider, Z_ARRAY(2, 200, 1) },
            { "Columns", _gParamType_Int, ZInt(24), Slider, Z_ARRAY(3, 400, 1) }
        ),
        Z_OUTPUTS(
            { "Output", _gParamType_Geometry }
        ),
        "create",
        "",
        ":/icons/node/tube.svg",
        ""
    );
}

#include "simple_geometry_common.h"

namespace zeno {

struct Cube : INode2 {
    zeno::NodeType type() const override { return zeno::Node_Normal; }
    float time() const override { return 1.0; }
    void clearCalcResults() override {}

    ZErrorCode apply(INodeData* ptrNodeData) override {
        zeno::vec3f Center = toVec3f(ptrNodeData->get_input2_vec3f("Center"));
        zeno::vec3f Size = toVec3f(ptrNodeData->get_input2_vec3f("Size"));
        zeno::vec3f Rotate = toVec3f(ptrNodeData->get_input2_vec3f("Rotate"));

        int x_division = ptrNodeData->get_input2_int("X Division");
        int y_division = ptrNodeData->get_input2_int("Y Division");
        int z_division = ptrNodeData->get_input2_int("Z Division");
        float uniform_scale = ptrNodeData->get_input2_float("Uniform Scale");
        std::string face_type = get_string_param(ptrNodeData, "Face Type");
        bool bCalcPointNormals = ptrNodeData->get_input2_bool("Point Normals");

        if (x_division < 2 || y_division < 2 || z_division < 2) {
            throw;
            ptrNodeData->report_error("the division should be greater than 2");
            return ZErr_ParamError;
        }

        bool bQuad = face_type == "Quadrilaterals";
        float xstep = 1.f / (x_division - 1), ystep = 1.f / (y_division - 1), zstep = 1.f / (z_division - 1);
        float xleft = -0.5f, xright = 0.5;
        float ybottom = -0.5f, ytop = 0.5;
        float zback = -0.5f, zfront = 0.5;

        float x, y, z = 0;

        int nPoints = 2 * (x_division * y_division) + (z_division - 2) * (2 * y_division + 2 * x_division - 4);
        int nFaces = 2 * (x_division - 1) * (y_division - 1) + 2 * (x_division - 1) * (z_division - 1) + 2 * (y_division - 1) * (z_division - 1);

        std::vector<zeno::vec3f> points, normals;
        std::vector<std::vector<int>> faces;
        points.resize(nPoints);
        faces.reserve(nFaces);
        if (bCalcPointNormals)
            normals.resize(nPoints);

        for (int z_div = 0; z_div < z_division; z_div++)
        {
            if (z_div == 0 || z_div == z_division - 1) {
                for (int y_div = 0; y_div < y_division; y_div++) {
                    for (int x_div = 0; x_div < x_division; x_div++) {

                        bool bFirstFace = z_div == 0;
                        zeno::vec3f pt(xleft + xstep * x_div, ybottom + ystep * y_div, zfront - zstep * z_div);

                        int nPrevPoints = 0;
                        if (!bFirstFace) {
                            nPrevPoints = x_division * y_division + (z_div - 1) * (2 * y_division + 2 * x_division - 4);
                        }

                        size_t idx = nPrevPoints + x_div % x_division + y_div * x_division;
                        points[idx] = pt;
                        if (bCalcPointNormals) {
                            float xcomp = (x_div == 0) ? -1 : ((x_div == x_division - 1) ? 1 : 0);
                            float ycomp = (y_div == 0) ? -1 : ((y_div == y_division - 1) ? 1 : 0);
                            float zcomp = (z_div == 0) ? 1 : ((z_div == z_division - 1) ? -1 : 0);
                            normals[idx] = normalize(zeno::vec3f(xcomp, ycomp, zcomp));
                        }

                        if (x_div > 0 && y_div > 0) {
                            int leftdown = nPrevPoints + (x_div - 1) % x_division + (y_div - 1) * x_division;
                            int rightdown = leftdown + 1;
                            int leftup = nPrevPoints + (x_div - 1) % x_division + y_div * x_division;
                            int rightup = (int)idx;

                            if (bFirstFace) {
                                if (bQuad) {
                                    std::vector<int> _newface = { leftdown, rightdown, rightup, leftup };
                                    faces.push_back(_newface);
                                }
                                else {
                                    std::vector<int> _newface = { leftdown, rightdown, leftup };
                                    faces.push_back(_newface);
                                    _newface = { rightdown, rightup, leftup };
                                    faces.push_back(_newface);
                                }
                            }
                            else {
                                if (bQuad) {
                                    std::vector<int> _newface = { leftdown, leftup, rightup, rightdown };
                                    faces.push_back(_newface);
                                }
                                else {
                                    std::vector<int> _newface = { leftdown, leftup, rightup };
                                    faces.push_back(_newface);
                                    _newface = { rightup, rightdown, leftdown };
                                    faces.push_back(_newface);
                                }
                            }
                        }
                    }
                }
            }

            if (z_div > 0)
            {
                for (int y_div = 0; y_div < y_division; y_div += (y_division - 1)) {
                    for (int x_div = 0; x_div < x_division; x_div++) {
                        zeno::vec3f pt(xleft + xstep * x_div, ybottom + ystep * y_div, zfront - zstep * z_div);
                        int nPrevPoints = x_division * y_division + (z_div - 1) * (2 * y_division + 2 * x_division - 4);

                        bool bTop = y_div == y_division - 1;
                        bool bLastFace = z_div == z_division - 1;

                        int idx = 0;
                        if (bLastFace) {
                            idx = nPrevPoints + x_div % x_division + y_div * x_division;
                        }
                        else {
                            idx = nPrevPoints + bTop * (x_division + (y_division - 2) * 2) + x_div;
                        }

                        points[idx] = pt;
                        if (bCalcPointNormals) {
                            float xcomp = (x_div == 0) ? -1 : ((x_div == x_division - 1) ? 1 : 0);
                            float ycomp = (y_div == 0) ? -1 : ((y_div == y_division - 1) ? 1 : 0);
                            float zcomp = (z_div == 0) ? 1 : ((z_div == z_division - 1) ? -1 : 0);
                            normals[idx] = normalize(zeno::vec3f(xcomp, ycomp, zcomp));
                        }

                        if (x_div > 0) {
                            int leftdown = 0;
                            int rightup = idx;
                            if (z_div > 1) {
                                int nPrevPrevPoints = x_division * y_division + (z_div - 2) * (2 * y_division + 2 * x_division - 4);
                                if (bTop) {
                                    leftdown = nPrevPrevPoints + x_division + y_division * 2 - 4 + x_div - 1;
                                }
                                else {
                                    leftdown = nPrevPrevPoints + x_div - 1;
                                }
                            }
                            else {
                                if (bTop) {
                                    leftdown = (x_div - 1) % x_division + y_div * x_division;
                                }
                                else {
                                    leftdown = x_div - 1;
                                }
                            }

                            int rightdown = leftdown + 1;
                            int leftup = idx - 1;

                            if (bTop) {
                                if (bQuad) {
                                    std::vector<int> _newface = { leftdown, rightdown, rightup, leftup };
                                    faces.push_back(_newface);
                                }
                                else {
                                    std::vector<int> _newface = { leftdown, rightdown, leftup };
                                    faces.push_back(_newface);
                                    _newface = { rightdown, rightup, leftup };
                                    faces.push_back(_newface);
                                }
                            }
                            else {
                                if (bQuad) {
                                    std::vector<int> _newface = { leftdown, leftup, rightup, rightdown };
                                    faces.push_back(_newface);
                                }
                                else {
                                    std::vector<int> _newface = { leftdown, leftup, rightup };
                                    faces.push_back(_newface);
                                    _newface = { rightdown, leftdown, rightup };
                                    faces.push_back(_newface);
                                }
                            }
                        }
                    }
                }
                for (int x_div = 0; x_div < x_division; x_div += (x_division - 1))
                {
                    for (int y_div = 0; y_div < y_division; y_div++) {
                        zeno::vec3f pt(xleft + xstep * x_div, ybottom + ystep * y_div, zfront - zstep * z_div);

                        bool bTop = y_div == y_division - 1;
                        bool bRight = x_div == x_division - 1;
                        bool bLastFace = z_div == z_division - 1;
                        int nPrevPoints = x_division * y_division + (z_div - 1) * (2 * y_division + 2 * x_division - 4);
                        int idx = 0;
                        if (bLastFace) {
                            idx = nPrevPoints + x_div % x_division + y_div * x_division;
                        }
                        else {
                            if (y_div == 0) {
                                idx = nPrevPoints + x_div * bRight;
                            }
                            else if (y_div == y_division - 1) {
                                idx = nPrevPoints + y_div * 2 + x_division - 2 + x_div * bRight;
                            }
                            else {
                                idx = nPrevPoints + y_div * 2 + x_division - 2 + bRight;
                            }
                        }

                        points[idx] = pt;
                        if (bCalcPointNormals) {
                            float xcomp = (x_div == 0) ? -1 : ((x_div == x_division - 1) ? 1 : 0);
                            float ycomp = (y_div == 0) ? -1 : ((y_div == y_division - 1) ? 1 : 0);
                            float zcomp = (z_div == 0) ? 1 : ((z_div == z_division - 1) ? -1 : 0);
                            normals[idx] = normalize(zeno::vec3f(xcomp, ycomp, zcomp));
                        }

                        if (y_div > 0) {
                            int leftdown = 0;
                            int nPrevPrevPoints = x_division * y_division + (z_div - 2) * (2 * y_division + 2 * x_division - 4);

                            int rightdown = 0;
                            int rightup = idx;
                            if (bLastFace) {
                                rightdown = nPrevPoints + x_div % x_division + (y_div - 1) * x_division;
                            }
                            else {
                                if (y_div == 1) {
                                    rightdown = bRight ? rightup - 2 : rightup - x_division;
                                }
                                else if (bTop && bRight) {
                                    rightdown = rightup - x_division;
                                }
                                else {
                                    rightdown = rightup - 2;
                                }
                            }

                            int leftup = 0;
                            if (z_div > 1) {
                                if (bRight) {
                                    if (bTop) {
                                        leftup = nPrevPrevPoints + x_division + y_division * 2 - 2 - 2 + x_div;
                                    }
                                    else {
                                        leftup = nPrevPrevPoints + y_div * 2 + x_division - 2 + 1;
                                    }
                                }
                                else {
                                    if (bTop) {
                                        leftup = nPrevPrevPoints + x_division + y_division * 2 - 2 - 2 + x_div;
                                    }
                                    else {
                                        leftup = nPrevPrevPoints + y_div * 2 + x_division - 2;
                                    }
                                }
                            }
                            else {
                                leftup = x_div % x_division + y_div * x_division;
                            }

                            if (z_div > 1) {
                                if (y_div == 1) {
                                    leftdown = bRight ? leftup - 2 : leftup - x_division;
                                }
                                else if (bTop && bRight) {
                                    leftdown = leftup - x_division;
                                }
                                else {
                                    leftdown = leftup - 2;
                                }
                            }
                            else {
                                leftdown = x_div % x_division + (y_div - 1) * x_division;
                            }

                            if (bRight) {
                                if (bQuad) {
                                    std::vector<int> _newface = { leftdown, rightdown, rightup, leftup };
                                    faces.push_back(_newface);
                                }
                                else {
                                    std::vector<int> _newface = { leftdown, rightdown, leftup };
                                    faces.push_back(_newface);
                                    _newface = { rightdown, rightup, leftup };
                                    faces.push_back(_newface);
                                }
                            }
                            else {
                                if (bQuad) {
                                    std::vector<int> _newface = { leftdown, leftup, rightup, rightdown };
                                    faces.push_back(_newface);
                                }
                                else {
                                    std::vector<int> _newface = { leftdown, leftup, rightup };
                                    faces.push_back(_newface);
                                    _newface = { rightdown, leftdown, rightup };
                                    faces.push_back(_newface);
                                }
                            }
                        }
                    }
                }
            }
        }

        glm::mat4 scale_matrix = glm::scale(glm::mat4(1.0), glm::vec3(uniform_scale * Size[0], uniform_scale * Size[1], uniform_scale * Size[2]));
        glm::mat4 translate = glm::translate(glm::mat4(1.0), glm::vec3(Center[0], Center[1], Center[2]));
        glm::mat4 rotation = calc_rotate_matrix(Rotate[0], Rotate[1], Rotate[2], Orientaion_ZX);
        glm::mat4 transform = translate * rotation * scale_matrix;
        for (size_t i = 0; i < points.size(); i++)
        {
            auto pt = points[i];
            glm::vec4 gp = transform * glm::vec4(pt[0], pt[1], pt[2], 1);
            points[i] = zeno::vec3f(gp.x, gp.y, gp.z);
            if (bCalcPointNormals) {
                auto nrm = normals[i];
                glm::vec4 gnrm = rotation * glm::vec4(nrm[0], nrm[1], nrm[2], 0);
                normals[i] = zeno::vec3f(gnrm.x, gnrm.y, gnrm.z);
            }
        }

        auto geo = create_GeometryObject(Topo_IndiceMesh2, !bQuad, to_glm_points(points), faces);
        if (bCalcPointNormals) {
            size_t outCount = 0;
            auto nrmData = convert_points_to_abi(normals, outCount);
            geo->create_attr_by_vec3(ATTR_POINT, "nrm", nrmData.get(), outCount);
        }
        ptrNodeData->set_output_object("Output", geo);
        return ZErr_OK;
    }
};

ZENDEFNODE_ABI(Cube,
    Z_INPUTS(
        { "Center", _gParamType_Vec3f, ZVec3f(0,0,0) },
        { "Size", _gParamType_Vec3f, ZVec3f(1,1,1) },
        { "Rotate", _gParamType_Vec3f, ZVec3f(0,0,0) },
        { "X Division", _gParamType_Int, ZInt(2), Lineedit },
        { "Y Division", _gParamType_Int, ZInt(2), Lineedit },
        { "Z Division", _gParamType_Int, ZInt(2), Lineedit },
        { "Uniform Scale", _gParamType_Float, ZFloat(1.0f), Lineedit },
        { "Face Type", _gParamType_String, ZString("Quadrilaterals"), Combobox, Z_STRING_ARRAY("Triangles", "Quadrilaterals") },
        { "Point Normals", _gParamType_Bool, ZInt(0), Checkbox }
    ),
    Z_OUTPUTS(
        { "Output", _gParamType_Geometry }
    ),
    "geometry",
    "create a cube",
    ":/icons/node/cube.svg",
    ""
);

} // namespace zeno


#include <vec.h>
#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include <inodedata.h>
#include <inodeimpl.h>
#include "api.h"
#include "zcommon.h"
#include <Windows.h>
#include "vecutil.h"
#include "typehelper.h"
#include "utils.h"
#include <vector>


namespace zeno {

    static std::vector<glm::vec3> to_glm_points(const std::vector<zeno::vec3f>& v) {
        std::vector<glm::vec3> r(v.size());
        for (size_t i = 0; i < v.size(); i++)
            r[i] = glm::vec3(v[i][0], v[i][1], v[i][2]);
        return r;
    }

    static glm::mat4 calc_rotate_matrix(
        float xangle,
        float yangle,
        float zangle,
        Rotate_Orientaion orientaion
    ) {
        float rad_x = xangle * (M_PI / 180.0);
        float rad_y = yangle * (M_PI / 180.0);
        float rad_z = zangle * (M_PI / 180.0);
#if 0
        switch (orientaion)
        {
        case Orientaion_XY: //绕x轴旋转90
            rad_x = (xangle + 90) * (M_PI / 180.0);
            break;
        case Orientaion_YZ: //绕z轴旋转-90
            rad_z = (zangle - 90) * (M_PI / 180.0);
            break;
        case Orientaion_ZX:
            break;//默认都是基于ZX平面
        }
#endif
        //这里构造的方式是基于列，和公式上的一样，所以看起来反过来了
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
                throw;// makeError<UnimplError>("the division should be greater than 2");
                ptrNodeData->report_error("the division should be greater than 2");
                return ZErr_ParamError;
            }

            bool bQuad = face_type == "Quadrilaterals";
            float xstep = 1.f / (x_division - 1), ystep = 1.f / (y_division - 1), zstep = 1.f / (z_division - 1);
            float xleft = -0.5f, xright = 0.5;
            float ybottom = -0.5f, ytop = 0.5;
            float zback = -0.5f, zfront = 0.5;

            float x, y, z = 0;
            float z_prev = zback;

            //TODO: nFaces需要考虑三角面的情况
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
                                //枚举以前所有z_div平面时已处理的顶点数，下同
                                nPrevPoints = x_division * y_division + (z_div - 1) * (2 * y_division + 2 * x_division - 4);
                            }

                            size_t idx = nPrevPoints + x_div % x_division + y_div * x_division;
                            points[idx] = pt;
                            //init normals
                            if (bCalcPointNormals) {
                                float xcomp = (x_div == 0) ? -1 : ((x_div == x_division - 1) ? 1 : 0);
                                float ycomp = (y_div == 0) ? -1 : ((y_div == y_division - 1) ? 1 : 0);
                                float zcomp = (z_div == 0) ? 1 : ((z_div == z_division - 1) ? -1 : 0);
                                normals[idx] = normalize(zeno::vec3f(xcomp, ycomp, zcomp));
                            }

                            if (x_div > 0 && y_div > 0) {
                                //current traversal point is rightup.
                                int leftdown = nPrevPoints + (x_div - 1) % x_division + (y_div - 1) * x_division;
                                int rightdown = leftdown + 1;
                                int leftup = nPrevPoints + (x_div - 1) % x_division + y_div * x_division;
                                int rightup = idx;

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
                    //x方向，处理x-z轴的顶面和底面。
                    for (int y_div = 0; y_div < y_division; y_div += (y_division - 1)) {
                        for (int x_div = 0; x_div < x_division; x_div++) {
                            zeno::vec3f pt(xleft + xstep * x_div, ybottom + ystep * y_div, zfront - zstep * z_div);
                            int nPrevPoints = x_division * y_division + (z_div - 1) * (2 * y_division + 2 * x_division - 4);

                            bool bTop = y_div == y_division - 1;
                            bool bLastFace = z_div == z_division - 1;

                            //计算当前节点的索引位置：
                            int idx = 0;
                            if (bLastFace) {
                                idx = nPrevPoints + x_div % x_division + y_div * x_division;
                            }
                            else {
                                idx = nPrevPoints + bTop * (x_division + (y_division - 2) * 2) + x_div;
                            }

                            points[idx] = pt;
                            //init normals
                            if (bCalcPointNormals) {
                                float xcomp = (x_div == 0) ? -1 : ((x_div == x_division - 1) ? 1 : 0);
                                float ycomp = (y_div == 0) ? -1 : ((y_div == y_division - 1) ? 1 : 0);
                                float zcomp = (z_div == 0) ? 1 : ((z_div == z_division - 1) ? -1 : 0);
                                normals[idx] = normalize(zeno::vec3f(xcomp, ycomp, zcomp));
                            }

                            //添加底部（往z正方向）的面
                            if (x_div > 0) {
                                int leftdown = 0;
                                int rightup = idx;
                                if (z_div > 1) {
                                    int nPrevPrevPoints = x_division * y_division + (z_div - 2) * (2 * y_division + 2 * x_division - 4);
                                    if (bTop) {
                                        leftdown = nPrevPrevPoints + x_division + y_division * 2 - 4/*两个重合点，以及左右上角*/ + x_div - 1;
                                    }
                                    else {
                                        leftdown = nPrevPrevPoints + x_div - 1/*因为是Leftdown，还需要往左偏移一个单位*/;
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
                    //y方向，处理y-z轴的侧面。
                    for (int x_div = 0; x_div < x_division; x_div += (x_division - 1))
                    {
                        for (int y_div = 0; y_div < y_division; y_div++) {
                            zeno::vec3f pt(xleft + xstep * x_div, ybottom + ystep * y_div, zfront - zstep * z_div);

                            //x正方向的那个侧面
                            bool bTop = y_div == y_division - 1;
                            bool bRight = x_div == x_division - 1;
                            bool bLastFace = z_div == z_division - 1;
                            int nPrevPoints = x_division * y_division + (z_div - 1) * (2 * y_division + 2 * x_division - 4);
                            //计算当前节点的索引位置：
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
                            //init normals
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
                                        rightdown = rightup - 2;    //就在正下方
                                    }
                                }

                                int leftup = 0;
                                if (z_div > 1) {
                                    if (bRight) {
                                        if (bTop) {
                                            leftup = nPrevPrevPoints + x_division + y_division * 2 - 2/*下方xy两个重合点*/ - 2/*最上面两个点*/ + x_div;
                                        }
                                        else {
                                            leftup = nPrevPrevPoints + y_div * 2 + x_division - 2 + 1;
                                        }
                                    }
                                    else {
                                        if (bTop) {
                                            leftup = nPrevPrevPoints + x_division + y_division * 2 - 2/*下方xy两个重合点*/ - 2/*最上面两个点*/ + x_div;
                                        }
                                        else {
                                            leftup = nPrevPrevPoints + y_div * 2 + x_division - 2;
                                        }
                                    }
                                }
                                else {
                                    leftup = x_div % x_division + y_div * x_division;
                                }

                                //x负方向的那个侧面
                                if (z_div > 1) {
                                    //根据leftup算leftdown
                                    if (y_div == 1) {
                                        leftdown = bRight ? leftup - 2 : leftup - x_division;
                                    }
                                    else if (bTop && bRight) {
                                        leftdown = leftup - x_division;
                                    }
                                    else {
                                        leftdown = leftup - 2;    //就在正下方
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


    struct Grid : INode2 {

        enum PlaneDirection {
            Dir_XY,
            Dir_YZ,
            Dir_ZX
        };

        DEF_OVERRIDE_FOR_INODE

        ZErrorCode apply(INodeData* ptrNodeData) override {
            zeno::vec3f Center = toVec3f(ptrNodeData->get_input2_vec3f("Center"));
            zeno::vec3f Rotate = toVec3f(ptrNodeData->get_input2_vec3f("Rotate"));
            zeno::vec2f Size = toVec2f(ptrNodeData->get_input2_vec2f("Size"));
            int Rows = ptrNodeData->get_input2_int("Rows");
            int Columns = ptrNodeData->get_input2_int("Columns");
            std::string face_type = get_string_param(ptrNodeData, "Face Type");
            std::string Direction = get_string_param(ptrNodeData, "Direction");
            bool bCalcPointNormals = ptrNodeData->get_input2_bool("Point Normal");

            if (Rows < 2 || Columns < 2) {
                //throw; //("the division should be greater than 2");
                return ZErr_ParamError;
            }

            float size1 = Size[0], size2 = Size[1];
            float step1 = size1 / (Rows - 1), step2 = size2 / (Columns - 1);
            float bottom1 = -size1 / 2, up1 = bottom1 + size1;
            float bottom2 = -size2 / 2, up2 = bottom2 + size2;
            bool bQuad = face_type == "Quadrilaterals";

            int nPoints = Rows * Columns;
            int nFaces = (Rows - 1) * (Columns - 1);
            if (!bQuad) {
                nFaces *= 2;
            }

            PlaneDirection dir;
            Rotate_Orientaion ori;
            if (Direction == "ZX") {
                dir = Dir_ZX;
                ori = Orientaion_ZX;
            }
            else if (Direction == "YZ") {
                dir = Dir_YZ;
                ori = Orientaion_YZ;
            }
            else if (Direction == "XY") {
                dir = Dir_XY;
                ori = Orientaion_XY;
            }
            else {
                //throw;// makeError<UnimplError>("Unknown Direction");
                return ZErr_ParamError;
            }

            std::vector<vec3f> points, normals;
            std::vector<std::vector<int>> faces;
            points.resize(nPoints);
            faces.reserve(nFaces);
            if (bCalcPointNormals)
                normals.resize(nPoints);

            for (size_t i = 0; i < Rows; i++) {
                for (size_t j = 0; j < Columns; j++) {
                    //zeno::vec3f pt(xleft + xstep * x_div, ybottom + ystep * y_div, zfront - zstep * z_div);
                    vec3f pt, nrm;
                    if (dir == Dir_ZX) {
                        pt = vec3f(bottom2 + step2 * j, 0, bottom1 + step1 * i);
                        nrm = vec3f(0, 1, 0);
                    }
                    else if (dir == Dir_YZ) {
                        pt = vec3f(0, bottom1 + step1 * i, bottom2 + step2 * j);
                        nrm = vec3f(1, 0, 0);
                    }
                    else {
                        pt = vec3f(bottom1 + step1 * i, bottom2 + step2 * j, 0);
                        nrm = vec3f(0, 0, -1);
                    }

                    int idx = i * Columns + j;
                    points[idx] = pt;
                    if (bCalcPointNormals)
                        normals[idx] = nrm;

                    if (j > 0 && i > 0) {
                        int ij = idx;
                        int ij_1 = idx - 1;
                        int i_1j = (i - 1) * Columns + j;
                        int i_1j_1 = i_1j - 1;

                        if (bQuad) {
                            std::vector<int> _face = { ij, i_1j, i_1j_1, ij_1 };
                            faces.push_back(_face);
                        }
                        else {
                            std::vector<int> _face = { ij, i_1j, i_1j_1 };
                            faces.push_back(_face);
                            _face = { ij, i_1j_1, ij_1 };
                            faces.push_back(_face);
                        }
                    }
                }
            }

            glm::mat4 translate = glm::translate(glm::mat4(1.0), glm::vec3(Center[0], Center[1], Center[2]));
            glm::mat4 rotation = calc_rotate_matrix(Rotate[0], Rotate[1], Rotate[2], ori);
            glm::mat4 transform = translate * rotation;
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

            auto geo = create_GeometryObject(Topo_HalfEdge, !bQuad, to_glm_points(points), faces);
            if (bCalcPointNormals) {
                size_t outCount = 0;
                auto nrmData = convert_points_to_abi(normals, outCount);
                geo->create_attr_by_vec3(ATTR_POINT, "nrm", nrmData.get(), outCount);
            }
            ptrNodeData->set_output_object("Output", std::move(geo));
            return ZErr_OK;
        }
    };

    ZENDEFNODE_ABI(Grid,
        Z_INPUTS(
            { "Center", _gParamType_Vec3f, ZVec3f(0,0,0) },
            { "Size", _gParamType_Vec2f, ZVec2f(1,1) },
            { "Rotate", _gParamType_Vec3f, ZVec3f(0,0,0) },
            { "Rows", _gParamType_Int, ZInt(2), Slider, Z_ARRAY(1, 100, 1) },
            { "Columns", _gParamType_Int, ZInt(2), Slider, Z_ARRAY(1, 100, 1) },
            { "Direction", _gParamType_String, ZString("ZX"), Combobox, Z_STRING_ARRAY("XY", "YZ", "ZX") },
            { "Uniform Scale", _gParamType_Float, ZFloat(1.0f), Lineedit },
            { "Face Type", _gParamType_String, ZString("Quadrilaterals"), Combobox, Z_STRING_ARRAY("Triangles", "Quadrilaterals") },
            { "Point Normal", _gParamType_Bool, ZInt(0), Checkbox }
        ),
        Z_OUTPUTS(
            { "Output", _gParamType_Geometry }
        ),
        "geometry",
        "create a cube",
        "",
        ""
    );


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

            { "Direction", _gParamType_String,
              ZString("Y Axis"),
              Combobox,
              Z_STRING_ARRAY("X Axis","Y Axis","Z Axis") },

            { "Rows", _gParamType_Int,
              ZInt(13),
              Slider,
              Z_ARRAY(3,100,1) },

            { "Columns", _gParamType_Int,
              ZInt(24),
              Slider,
              Z_ARRAY(2,100,1) },

            { "Face Type", _gParamType_String,
              ZString("Quadrilaterals"),
              Combobox,
              Z_STRING_ARRAY("Triangles","Quadrilaterals") }
        ),

        Z_OUTPUTS(
            { "Output", _gParamType_Geometry }
        ),

        "create",
        "",
        ":/icons/node/sphere.svg",
        ""
    );


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

            float segments = ptrNodeData->get_input2_float("Segments");
            float radius = ptrNodeData->get_input2_float("Radius");

            std::string arcType = get_string_param(ptrNodeData, "Arc Type");

            Vec2i arcAngleABI = ptrNodeData->get_input2_vec2i("Arc Angle");
            zeno::vec2f arcAngle((float)arcAngleABI.x, (float)arcAngleABI.y);

            std::string direction = get_string_param(ptrNodeData, "ZX");
            bool bCalcPointNormals = ptrNodeData->get_input2_bool("Point Normals");

            if (segments <= 0 || radius < 0)
                return ZErr_ParamError;

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

            size_t pointNumber = 0, faceNumber = 0;

            if (arcType == "Closed") {
                if (segments == 1) {
                    pointNumber = 1;
                    faceNumber = 0;
                }
                else {
                    pointNumber = segments + 1;
                    faceNumber = segments;
                }
            }
            else if (arcType == "Open Arc") {
                pointNumber = segments + 1;
                faceNumber = 0;
            }
            else if (arcType == "Sliced Arc") {
                pointNumber = segments + 2;
                faceNumber = segments;
            }

            std::vector<zeno::vec3f> points, normals;
            std::vector<std::vector<int>> faces;

            faces.reserve(faceNumber);
            points.resize(pointNumber);
            if (bCalcPointNormals)
                normals.resize(pointNumber);

            if (arcType == "Closed") {

                if (segments == 1) {
                    points.push_back(Center);

                    auto geo = create_GeometryObject(Topo_HalfEdge, true, to_glm_points(points), faces);
                    ptrNodeData->set_output_object("Output", geo);
                    return ZErr_OK;
                }

                points[0] = Center;

                for (int i = 1; i < pointNumber; ++i) {
                    float rad = 2.0f * M_PI * (i - 1) / (pointNumber - 1);
                    zeno::vec3f pt, nrm;

                    if (dir == Dir_ZX) {
                        pt = { cos(rad) * radius, 0, -sin(rad) * radius };
                        nrm = { 0,1,0 };
                    }
                    else if (dir == Dir_YZ) {
                        pt = { 0, cos(rad) * radius, -sin(rad) * radius };
                        nrm = { 1,0,0 };
                    }
                    else {
                        pt = { cos(rad) * radius, -sin(rad) * radius, 0 };
                        nrm = { 0,0,1 };
                    }

                    points[i] = pt;
                    if (bCalcPointNormals) normals[i] = nrm;

                    if (i > 1)
                        faces.push_back({ 0, i - 1, i });
                }

                faces.push_back({ 0, (int)pointNumber - 1, 1 });
            }
            else if (arcType == "Open Arc") {

                float startAngle = arcAngle[0];
                float arcRange = arcAngle[1] - startAngle;
                std::vector<int> ptIndice;

                for (int i = 0; i < pointNumber; ++i) {
                    float rad = glm::radians(startAngle + arcRange * i / (pointNumber - 1));
                    zeno::vec3f pt;

                    if (dir == Dir_ZX)
                        pt = { cos(rad) * radius,0,-sin(rad) * radius };
                    else if (dir == Dir_YZ)
                        pt = { 0,cos(rad) * radius,-sin(rad) * radius };
                    else
                        pt = { cos(rad) * radius,-sin(rad) * radius,0 };

                    points[i] = pt;
                    ptIndice.push_back(i);
                }

                faces.push_back(ptIndice);
                bCalcPointNormals = false;
            }
            else if (arcType == "Sliced Arc") {

                points[0] = Center;

                float startAngle = arcAngle[0];
                float arcRange = arcAngle[1] - startAngle;

                for (int i = 1; i < pointNumber; ++i) {
                    float rad = glm::radians(startAngle + arcRange * (i - 1) / (pointNumber - 2));
                    zeno::vec3f pt, nrm;

                    if (dir == Dir_ZX) {
                        pt = { cos(rad) * radius,0,-sin(rad) * radius };
                        nrm = { 0,1,0 };
                    }
                    else if (dir == Dir_YZ) {
                        pt = { 0,cos(rad) * radius,-sin(rad) * radius };
                        nrm = { 1,0,0 };
                    }
                    else {
                        pt = { cos(rad) * radius,-sin(rad) * radius,0 };
                        nrm = { 0,0,1 };
                    }

                    points[i] = pt;
                    if (bCalcPointNormals) normals[i] = nrm;

                    if (i > 1)
                        faces.push_back({ 0,i - 1,i });
                }
            }

            glm::mat4 translate = glm::translate(glm::mat4(1.0f),
                glm::vec3(Center[0], Center[1], Center[2]));
            glm::mat4 rotation = calc_rotate_matrix(Rotate[0], Rotate[1], Rotate[2], ori);
            glm::mat4 transform = translate * rotation;

            for (size_t i = arcType == "Open Arc" ? 0 : 1; i < points.size(); ++i) {
                auto& pt = points[i];
                glm::vec4 gp = transform * glm::vec4(pt[0], pt[1], pt[2], 1);
                pt = { gp.x,gp.y,gp.z };

                if (bCalcPointNormals) {
                    auto& nrm = normals[i];
                    glm::vec4 gnrm = rotation * glm::vec4(nrm[0], nrm[1], nrm[2], 0);
                    nrm = { gnrm.x,gnrm.y,gnrm.z };
                }
            }

            auto geo = create_GeometryObject(Topo_HalfEdge, true, to_glm_points(points), faces);

            // pos
            {
                size_t outCount = 0;
                auto abiPts = convert_points_to_abi(points, outCount);
                geo->create_attr_by_vec3(ATTR_POINT, "pos", abiPts.get(), outCount);
            }

            // nrm
            if (bCalcPointNormals) {
                size_t outCount = 0;
                auto abiNrms = convert_points_to_abi(normals, outCount);
                geo->create_attr_by_vec3(ATTR_POINT, "nrm", abiNrms.get(), outCount);
            }

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

            { "Arc Type", _gParamType_String,
              ZString("Closed"),
              Combobox,
              Z_STRING_ARRAY("Closed", "Open Arc", "Sliced Arc") },

            { "Arc Angle", _gParamType_Vec2f, ZVec2f(0,120) },

            { "Direction", _gParamType_String,
              ZString("ZX"),
              Combobox,
              Z_STRING_ARRAY("XY","YZ","ZX") },

            { "Point Normals", _gParamType_Bool, ZInt(0), Checkbox }
        ),

        Z_OUTPUTS(
            { "Output", _gParamType_Geometry }
        ),

        "create",
        "",
        ":/icons/node/circle.svg",
        ""
    );


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

            if (npoints <= 0) {
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

}

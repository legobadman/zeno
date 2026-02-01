#include <vec.h>
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
        zeno::NodeType type() const override {
            return zeno::Node_Normal;
        }
        void getIconResource(char* recv, size_t cap) override {
            const char* icon = ":/icons/node/cube.svg";
            strcpy(recv, icon);
            recv[strlen(icon)] = '\0';
        }
        void getBackgroundClr(char* recv, size_t cap) override {}
        float time() const override { return 1.0; }
        void clearCalcResults() override {}

        void apply(INodeData* ptrNodeData) override {
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

            auto geo = create_GeometryObject(Topo_HalfEdge, !bQuad, points, faces);
            if (bCalcPointNormals) {
                size_t outCount = 0;
                auto nrmData = convert_points_to_abi(normals, outCount);
                geo->create_attr_by_vec3(ATTR_POINT, "nrm", nrmData.get(), outCount);
            }
            ptrNodeData->set_output_object("Output", geo);
        }
    };

    ZENDEFNODE_ABI(Cube,
        Z_INPUTS(
            { "Center", _gParamType_Vec3f, ZVec3f(0,0,0) },
            { "Size", _gParamType_Vec3f, ZVec3f(1,1,1) },
            { "Rotate", _gParamType_Vec3f, ZVec3f(0,0,0) },
            { "X Division", _gParamType_Int, ZInt(2), ctrl_Lineedit },
            { "Y Division", _gParamType_Int, ZInt(2), ctrl_Lineedit },
            { "Z Division", _gParamType_Int, ZInt(2), ctrl_Lineedit },
            { "Uniform Scale", _gParamType_Float, ZFloat(1.0f), ctrl_Lineedit },
            { "Face Type", _gParamType_String, ZString("Quadrilaterals"), ctrl_Combobox, Z_STRING_ARRAY("Triangles", "Quadrilaterals") },
            { "Point Normals", _gParamType_Bool, ZInt(1), ctrl_Checkbox }
            ),
        Z_OUTPUTS(
            { "Output", _gParamType_Geometry }
        ),
        "create",
        "create a cube"
    );

}

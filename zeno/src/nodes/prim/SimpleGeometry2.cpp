#include <zeno/zeno.h>
#include <zeno/core/reflectdef.h>
#include <zeno/types/GeometryObject.h>
#include <zeno/geo/geometryutil.h>
#include "glm/gtc/matrix_transform.hpp"
#include "zeno_types/reflect/reflection.generated.hpp"


#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


namespace zeno {

    using namespace zeno::reflect;

    struct ZDEFNODE() Cube : INode {

        ReflectCustomUI m_uilayout = {
            _Group {
                {"x_division", ParamPrimitive("X Division")},
                {"y_division", ParamPrimitive("Y Division")},
                {"z_division", ParamPrimitive("Z Division")},
                {"uniform_scale", ParamPrimitive("Uniform Scale")},
                {"face_type", ParamPrimitive("Face Type", "Quadrilaterals", Combobox, std::vector<std::string>{"Triangles", "Quadrilaterals"})},
                {"bCalcPointNormals", ParamPrimitive("Point Normals")},
            },
            _Group {
                {"", ParamObject("Output")},
            }
        };

        std::shared_ptr<GeometryObject> apply(
            zeno::vec3f Center = zeno::vec3f({ 0,0,0 }),
            zeno::vec3f Size = zeno::vec3f({ 1,1,1 }),
            zeno::vec3f Rotate = zeno::vec3f({ 0,0,0 }),
            int x_division = 2,
            int y_division = 2,
            int z_division = 2,
            float uniform_scale = 1.f,
            std::string face_type = "Quadrilaterals",
            bool bCalcPointNormals = false
        )
        {
            if (x_division < 2 || y_division < 2 || z_division < 2) {
                throw makeError<UnimplError>("the division should be greater than 2");
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

            auto geo = std::make_shared<zeno::GeometryObject>(!bQuad, nPoints, nFaces);
            std::vector<vec3f> points, normals;
            points.resize(nPoints);
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
                            geo->initpoint(idx);
                            //init normals
                            if (bCalcPointNormals) {
                                float xcomp = (x_div == 0) ? -1 : ((x_div == x_division - 1) ? 1 : 0);
                                float ycomp = (y_div == 0) ? -1 : ((y_div == y_division - 1) ? 1 : 0);
                                float zcomp = (z_div == 0) ? -1 : ((z_div == z_division - 1) ? 1 : 0);
                                normals[idx] = normalize(zeno::vec3f(xcomp, ycomp, zcomp));
                            }

                            if (x_div > 0 && y_div > 0) {
                                //current traversal point is rightup.
                                size_t leftdown = nPrevPoints + (x_div - 1) % x_division + (y_div - 1) * x_division;
                                size_t rightdown = leftdown + 1;
                                size_t leftup = nPrevPoints + (x_div - 1) % x_division + y_div * x_division;
                                size_t rightup = idx;

                                if (bFirstFace) {
                                    if (bQuad) {
                                        geo->addface({ leftdown, rightdown, rightup, leftup });
                                    }
                                    else {
                                        geo->addface({ leftdown, rightdown, leftup });
                                        geo->addface({ rightdown, rightup, leftup });
                                    }
                                }
                                else {
                                    if (bQuad) {
                                        geo->addface({ leftdown, leftup, rightup, rightdown });
                                    }
                                    else {
                                        geo->addface({ leftdown, leftup, rightup });
                                        geo->addface({ rightup, rightdown, leftdown });
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
                            geo->initpoint(idx);
                            //init normals
                            if (bCalcPointNormals) {
                                float xcomp = (x_div == 0) ? -1 : ((x_div == x_division - 1) ? 1 : 0);
                                float ycomp = (y_div == 0) ? -1 : ((y_div == y_division - 1) ? 1 : 0);
                                float zcomp = (z_div == 0) ? -1 : ((z_div == z_division - 1) ? 1 : 0);
                                normals[idx] = normalize(zeno::vec3f(xcomp, ycomp, zcomp));
                            }

                            //添加底部（往z正方向）的面
                            if (x_div > 0) {
                                size_t leftdown = 0;
                                size_t rightup = idx;
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

                                size_t rightdown = leftdown + 1;
                                size_t leftup = idx - 1;

                                if (bTop) {
                                    if (bQuad) {
                                        geo->addface({ leftdown, rightdown, rightup, leftup });
                                    }
                                    else {
                                        geo->addface({ leftdown, rightdown, leftup });
                                        geo->addface({ rightdown, rightup, leftup });
                                    }
                                }
                                else {
                                    if (bQuad) {
                                        geo->addface({ leftdown, leftup, rightup, rightdown });
                                    }
                                    else {
                                        geo->addface({ leftdown, leftup, rightup });
                                        geo->addface({ rightdown, leftdown, rightup });
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
                            geo->initpoint(idx);
                            //init normals
                            if (bCalcPointNormals) {
                                float xcomp = (x_div == 0) ? -1 : ((x_div == x_division - 1) ? 1 : 0);
                                float ycomp = (y_div == 0) ? -1 : ((y_div == y_division - 1) ? 1 : 0);
                                float zcomp = (z_div == 0) ? -1 : ((z_div == z_division - 1) ? 1 : 0);
                                normals[idx] = normalize(zeno::vec3f(xcomp, ycomp, zcomp));
                            }

                            if (y_div > 0) {
                                size_t leftdown = 0;
                                size_t nPrevPrevPoints = x_division * y_division + (z_div - 2) * (2 * y_division + 2 * x_division - 4);

                                size_t rightdown = 0;
                                size_t rightup = idx;
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

                                size_t leftup = 0;
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
                                        geo->addface({ leftdown, rightdown, rightup, leftup });
                                    }
                                    else {
                                        geo->addface({ leftdown, rightdown, leftup });
                                        geo->addface({ rightdown, rightup, leftup });
                                    }
                                }
                                else {
                                    if (bQuad) {
                                        geo->addface({ leftdown, leftup, rightup, rightdown });
                                    }
                                    else {
                                        geo->addface({ leftdown, leftup, rightup });
                                        geo->addface({ rightdown, leftdown, rightup });
                                    }
                                }
                            }
                        }
                    }
                }
            }

            glm::mat4 scale_matrix = glm::scale(glm::mat4(1.0), glm::vec3(uniform_scale * Size[0], uniform_scale * Size[1], uniform_scale * Size[2]));
            glm::mat4 translate = glm::translate(glm::mat4(1.0), glm::vec3(Center[0], Center[1], Center[2]));
            glm::mat4 rotation = calc_rotate_matrix(Rotate[0], Rotate[1], Rotate[2], "");
            glm::mat4 transform = translate * rotation * scale_matrix;
            for (size_t i = 0; i < points.size(); i++)
            {
                auto pt = points[i];
                glm::vec4 gp = transform * glm::vec4(pt[0], pt[1], pt[2], 1);
                points[i] = zeno::vec3f(gp.x, gp.y, gp.z);
                //todo: normal.
            }

            geo->create_attr_value_by_vec(ATTR_POINT, "pos", points);
            if (bCalcPointNormals)
                geo->create_attr_value_by_vec(ATTR_POINT, "nrm", normals);
            return geo;
        }
    };

    struct ZDEFNODE() Disk : INode {
        std::shared_ptr<GeometryObject> apply(
            zeno::vec3f Position,
            zeno::vec3f Scale,
            zeno::vec3f Rotate,
            bool HasNormal = false,
            bool HasVertUV = false,
            bool IsFlipFace = false,
            float radius = 1.f,
            int divisions = 32
        ) {
            auto geo = std::make_shared<zeno::GeometryObject>();
            return geo;
        }
    };

    struct ZDEFNODE() Grid : INode {

        enum PlaneDirection {
            Dir_XY,
            Dir_YZ,
            Dir_ZX
        };

        ReflectCustomUI m_uilayout = {
            _Group {
                {"face_type", ParamPrimitive("Face Type", "Quadrilaterals", Combobox, std::vector<std::string>{"Triangles", "Quadrilaterals"})},
                {"Direction", ParamPrimitive("Direction", "ZX", Combobox, std::vector<std::string>{"XY", "YZ", "ZX"})},
                {"bCalcPointNormals", ParamPrimitive("Point Normals")},
            },
            _Group {
                {"", ParamObject("Output")},
            }
        };

        std::shared_ptr<GeometryObject> apply(
            zeno::vec3f Center = zeno::vec3f({ 0,0,0 }),
            zeno::vec3f Rotate = zeno::vec3f({ 0,0,0 }),
            zeno::vec2f Size = zeno::vec2f({ 1,1 }),
            int Rows = 2,
            int Columns = 2,
            std::string face_type = "Quadrilaterals",
            std::string Direction = "ZX",
            bool bCalcPointNormals = false
        ) {
            if (Rows < 2 || Columns < 2) {
                throw makeError<UnimplError>("the division should be greater than 2");
            }

            float size1 = Size[0], size2 = Size[1];
            float step1 = size1 / (Rows - 1), step2 = size2 / (Columns - 1);
            float bottom1 = - size1 / 2, up1 = bottom1 + size1;
            float bottom2 = - size2 / 2, up2 = bottom2 + size2;
            bool bQuad = face_type == "Quadrilaterals";

            int nPoints = Rows * Columns;
            int nFaces = (Rows - 1) * (Columns - 1);
            if (!bQuad) {
                nFaces *= 2;
            }

            PlaneDirection dir;
            if (Direction == "ZX") {
                dir = Dir_ZX;
            }
            else if (Direction == "YZ") {
                dir = Dir_YZ;
            }
            else if (Direction == "XY") {
                dir = Dir_XY;
            }
            else {
                throw makeError<UnimplError>("Unknown Direction");
            }

            auto geo = std::make_shared<zeno::GeometryObject>(!bQuad, nPoints, nFaces);
            std::vector<vec3f> points, normals;
            points.resize(nPoints);
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

                    size_t idx = i * Columns + j;
                    points[idx] = pt;
                    geo->initpoint(idx);
                    if (bCalcPointNormals)
                        normals[idx] = nrm;

                    if (j > 0 && i > 0) {
                        size_t ij = idx;
                        size_t ij_1 = idx - 1;
                        size_t i_1j = (i - 1) * Columns + j;
                        size_t i_1j_1 = i_1j - 1;

                        if (bQuad) {
                            geo->addface({ ij, i_1j, i_1j_1, ij_1 });
                        }
                        else {
                            geo->addface({ ij, i_1j, i_1j_1 });
                            geo->addface({ ij, i_1j_1, ij_1 });
                        }
                    }
                }
            }

            glm::mat4 translate = glm::translate(glm::mat4(1.0), glm::vec3(Center[0], Center[1], Center[2]));
            glm::mat4 rotation = calc_rotate_matrix(Rotate[0], Rotate[1], Rotate[2], Direction);
            glm::mat4 transform = translate * rotation;
            for (size_t i = 0; i < points.size(); i++)
            {
                auto pt = points[i];
                glm::vec4 gp = transform * glm::vec4(pt[0], pt[1], pt[2], 1);
                points[i] = zeno::vec3f(gp.x, gp.y, gp.z);
                //todo: normal.
            }

            geo->create_attr_value_by_vec(ATTR_POINT, "pos", points);
            if (bCalcPointNormals)
                geo->create_attr_value_by_vec(ATTR_POINT, "nrm", normals);
            return geo;
        }
    };

    struct ZDEFNODE() Tube : INode {

        ReflectCustomUI m_uilayout = {
            _Group {
                {"up_radius", ParamPrimitive("Up Radius")},
                {"down_radius", ParamPrimitive("Down Radius")},
                {"uniform_scale", ParamPrimitive("Uniform Scale")},
                {"face_type", ParamPrimitive("Face Type", "Quadrilaterals", Combobox, std::vector<std::string>{"Triangles", "Quadrilaterals"})},
                {"Direction", ParamPrimitive("Direction", "X Axis", Combobox, std::vector<std::string>{"X Axis", "Y Axis", "Z Axis"})},
                {"bCalcPointNormals", ParamPrimitive("Point Normals")},
                {"end_caps", ParamPrimitive("End Caps")},
            },
            _Group {
                {"", ParamObject("Output")},
            }
        };

        std::shared_ptr<GeometryObject> apply(
            zeno::vec3f Center,
            zeno::vec3f Rotate,
            float up_radius = 1.f,
            float down_radius = 1.f,
            float uniform_scale = 1.f,
            float Height = 1.0f,
            int Rows = 2,
            int Columns = 12,
            std::string Direction = "Y Axis",
            std::string face_type = "Quadrilaterals",
            bool bCalcPointNormals = false,
            bool end_caps = false
        )
        {
            bool bQuad = face_type == "Quadrilaterals";
            int nPoints = Rows * Columns;
            int nFaces = (Rows - 1) * Columns;    //暂不考虑end_caps
            if (!bQuad) {
                nFaces *= 2;
            }

            if (end_caps) {
                nFaces += 2;
            }
            if (!bQuad && end_caps) {
                //如果是三角形，就让中轴线顶部和底部多两个顶点，然后和顶部（底部）各个点形成三角形
                nPoints += 2;
            }

            auto geo = std::make_shared<zeno::GeometryObject>(!bQuad, nPoints, nFaces);
            std::vector<vec3f> points, normals;
            points.resize(nPoints);
            if (bCalcPointNormals)
                normals.resize(nPoints);

            float up_y = Height / 2.0f, down_y = -Height / 2.0f;

            if (end_caps) {
                //先把顶部和底部两个面加上
                if (bQuad) {
                    std::vector<size_t> up_pts, down_pts;
                    for (int col = 0; col < Columns; col++)
                    {
                        size_t up_idx = col;
                        geo->initpoint(up_idx);
                        up_pts.push_back(up_idx);
                    }
                    for (int col = Columns - 1; col >= 0; col--) {
                        size_t down_idx = (Rows - 1) * Columns + col;
                        geo->initpoint(down_idx);
                        down_pts.push_back(down_idx);
                    }
                    geo->addface(up_pts);
                    geo->addface(down_pts);
                }
                else {
                    points[0] = vec3f(0, up_y, 0);
                    geo->initpoint(0);
                    points[1] = vec3f(0, down_y, 0);
                    geo->initpoint(1);
                    for (int col = 0; col < Columns; col++)
                    {
                        size_t idx = col + 2;
                        geo->initpoint(idx);
                        if (col > 0) {
                            geo->addface({ 0, idx - 1, idx });
                        }
                        if (col == Columns - 1) {
                            geo->addface({ 0, idx, 2 });
                        }
                    }
                    for (int col = 0; col < Columns; col++)
                    {
                        size_t idx = (Rows - 1) * Columns + col + 2;
                        geo->initpoint(idx);
                        if (col > 0) {
                            geo->addface({ 1, idx, idx - 1 });
                        }
                        if (col == Columns - 1) {
                            size_t last_start = (Rows - 1) * Columns + 2;
                            geo->addface({ 1, last_start, idx});
                        }
                    }
                }
            }

            for (int row = 0; row < Rows; row++)
            {
                //指向侧（斜）面外部
                float tan_belta = (down_radius - up_radius) / Height;

                for (int col = 0; col < Columns; col++)
                {
                    float rad = 2.0f * M_PI * col / Columns;
                    float sin_a = sin(rad), cos_a = cos(rad);
                    const float up_x = up_radius * cos_a, up_z = -1 * up_radius * sin_a;
                    const float down_x = down_radius * cos_a, down_z = -1 * down_radius * sin_a;
                    vec3f up_pos(up_x, up_y, up_z);
                    vec3f down_pos(down_x, down_y, down_z);

                    size_t idx = row * Columns + col;
                    if (!bQuad && end_caps) {
                        idx += 2;       //三角面另外加上中轴线顶部和底部两个点
                    }

                    vec3f pt;
                    if (Direction == "Y Axis") {
                        pt = (float)row / (Rows - 1) * (down_pos - up_pos) + up_pos;
                    }
                    else {
                        throw;
                    }

                    points[idx] = pt;
                    geo->initpoint(idx);
                    if (bCalcPointNormals) {
                        normals[idx] = normalize(vec3f(cos_a, tan_belta * (sin_a * sin_a + cos_a * cos_a), -1 * sin_a));
                    }

                    if (row > 0 && col > 0) {
                        size_t right_bottom = idx;
                        size_t left_bottom = right_bottom - 1;
                        size_t right_top = right_bottom - Columns;
                        size_t left_top = right_top - 1;

                        if (bQuad) {
                            geo->addface({ left_top, left_bottom, right_bottom, right_top });
                        }
                        else {
                            geo->addface({ left_top, left_bottom, right_bottom });
                            geo->addface({ left_top, right_bottom, right_top });
                        }

                        if (col == Columns - 1) {
                            //连接这一圈最后一个点和起始点
                            right_bottom = idx - Columns + 1;
                            left_bottom = idx;
                            left_top = left_bottom - Columns;
                            right_top = right_bottom - Columns;

                            if (bQuad) {
                                geo->addface({ left_top, left_bottom, right_bottom, right_top });
                            }
                            else {
                                geo->addface({ left_top, left_bottom, right_bottom });
                                geo->addface({ left_top, right_bottom, right_top });
                            }
                        }
                    }
                }
            }
            geo->create_attr_value_by_vec(ATTR_POINT, "pos", points);
            if (bCalcPointNormals)
                geo->create_attr_value_by_vec(ATTR_POINT, "nrm", normals);
            return geo;
        }
    };

    struct ZDEFNODE() Sphere : INode {

        enum AxisDirection {
            Y_Axis,
            X_Axis,
            Z_Axis
        };

        ReflectCustomUI m_uilayout = {
            _Group {
                {"uniform_scale", ParamPrimitive("Uniform Scale")},
                {"face_type", ParamPrimitive("Face Type", "Quadrilaterals", Combobox, std::vector<std::string>{"Triangles", "Quadrilaterals"})},
                {"Direction", ParamPrimitive("Direction", "X Axis", Combobox, std::vector<std::string>{"X Axis", "Y Axis", "Z Axis"})},
            },
            _Group {
                {"", ParamObject("Output")},
            }
        };

        std::shared_ptr<GeometryObject> apply(
            zeno::vec3f Center,
            zeno::vec3f Rotate,
            zeno::vec3f Radius = zeno::vec3f(1.f, 1.f, 1.f),
            float uniform_scale = 1.f,
            std::string Direction = "Y Axis",
            int Rows = 13,
            int Columns = 24,
            std::string face_type = "Quadrilaterals")
        {
            bool bQuad = face_type == "Quadrilaterals";

            if (Rows < 3) {
                throw;
            }

            int nPoints = 2 + (Rows - 2) * Columns;
            int nFaces = 0;
            if (bQuad) {
                nFaces = (Rows - 1) * Columns;
            }
            else {
                nFaces = Columns * 2 + (Rows - 3) * Columns * 2;
            }

            float Rx = Radius[0], Ry = Radius[1], Rz = Radius[2];
            if (Rx <= 0 || Ry <= 0 || Rz <= 0) {
                throw;
            }

            AxisDirection dir;
            if (Direction == "Y Axis") {
                dir = Y_Axis;
            }
            else if (Direction == "X Axis") {
                dir = X_Axis;
            }
            else if (Direction == "Z Axis") {
                dir = Z_Axis;
            }
            else {
                throw;
            }

            std::vector<vec3f> points;
            points.resize(nPoints);

            auto geo = std::make_shared<zeno::GeometryObject>(!bQuad, nPoints, nFaces);

            //先加顶部和底部两个顶点
            vec3f topPos;
            if (dir == Y_Axis) {
                topPos = vec3f(0, 1, 0);
            }
            else if (dir == X_Axis) {
                topPos = vec3f(1, 0, 0);
            }
            else if (dir == Z_Axis) {
                topPos = vec3f(0, 0, 1);
            }

            int idx = 0;
            points[idx] = topPos;
            geo->initpoint(idx);

            vec3f bottomPos;
            if (dir == Y_Axis) {
                bottomPos = vec3f(0, -1, 0);
            }
            else if (dir == X_Axis) {
                bottomPos = vec3f(-1, 0, 0);
            }
            else if (dir == Z_Axis) {
                bottomPos = vec3f(0, 0, -1);
            }
            idx = 1;    //兼容houdini
            points[idx] = bottomPos;
            geo->initpoint(idx);

            float x, y, z;

            for (int row = 1; row < Rows - 1; row++)
            {
                float v = (float)row / (float)(Rows - 1);
                float theta = M_PI * v;
                if (dir == Y_Axis) y = cos(theta);
                else if (dir == X_Axis) x = cos(theta);
                else if (dir == Z_Axis) z = cos(theta);

                //col = 0, 从x轴正方向开始转圈圈
                size_t startIdx = 2 + (row - 1) * Columns;
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
                    else if (dir == X_Axis) {
                        y = sin(theta) * cos(phi);
                        z = -sin(theta) * sin(phi);
                    }

                    vec3f pt = vec3f(x, y, z);

                    size_t idx = 2/*顶部底部两个点*/ + (row - 1) * Columns + col;
                    geo->initpoint(idx);
                    points[idx] = pt;
                    if (col > 0) {
                        if (row == 1) {
                            //与顶部顶点构成三角面
                            geo->addface({ 0, idx - 1, idx });
                            if (col == Columns - 1) {
                                geo->addface({ 0, idx, startIdx });
                            }
                        }
                        else {
                            size_t rightdown = idx;
                            size_t leftdown = rightdown - 1;
                            size_t rightup = 2/*顶部底部两个点*/ + (row - 2) * Columns + col;
                            size_t leftup = rightup - 1;
                            if (bQuad) {
                                geo->addface({ rightdown, rightup, leftup, leftdown });
                            }
                            else {
                                geo->addface({ rightdown, rightup, leftup });
                                geo->addface({ rightdown, leftup, leftdown });
                            }

                            if (col == Columns - 1) {
                                leftup = rightup;
                                leftdown = rightdown;
                                rightdown = startIdx;
                                rightup = 2 + (row - 2) * Columns;

                                if (bQuad) {
                                    geo->addface({ rightdown, rightup, leftup, leftdown });
                                }
                                else {
                                    geo->addface({ rightdown, rightup, leftup });
                                    geo->addface({ rightdown, leftup, leftdown });
                                }
                            }

                            if (row == Rows - 2) {
                                //与底部顶点构成三角面
                                geo->addface({ idx, idx - 1, 1 });
                                if (col == Columns - 1) {
                                    geo->addface({ 1, startIdx, idx });
                                }
                            }
                        }
                    }
                }
            }

            glm::mat4 scale_matrix = glm::scale(glm::mat4(1.0), glm::vec3(uniform_scale * Radius[0], uniform_scale * Radius[1], uniform_scale * Radius[2]));
            glm::mat4 translate = glm::translate(glm::mat4(1.0), glm::vec3(Center[0], Center[1], Center[2]));
            glm::mat4 rotation = calc_rotate_matrix(Rotate[0], Rotate[1], Rotate[2], Direction);
            glm::mat4 transform = translate * rotation * scale_matrix;
            for (size_t i = 0; i < points.size(); i++)
            {
                auto pt = points[i];
                glm::vec4 gp = transform * glm::vec4(pt[0], pt[1], pt[2], 1);
                points[i] = zeno::vec3f(gp.x, gp.y, gp.z);
                //todo: normal.
            }

            geo->create_attr_value_by_vec(ATTR_POINT, "pos", points);
            return geo;
        }
    };

    struct ZDEFNODE() Cone : INode {
        std::shared_ptr<GeometryObject> apply(
            zeno::vec3f Position,
            zeno::vec3f Scale,
            float radius = 1.f,
            float height = 2.f,
            int lons = 32
        ) {
            auto geo = std::make_shared<zeno::GeometryObject>();
            return geo;
        }
    };

    struct ZDEFNODE() Torus : INode {
        std::shared_ptr<GeometryObject> apply(
            zeno::vec3f Position,
            zeno::vec3f Rotate,
            float MajorRadius = 1.f,
            float MinorRadius = 0.25f,
            bool HasNormal = false,
            bool HasVertUV = false,
            int MajorSegment = 48,
            int MinorSegment = 12,
            bool quads = false
        ) {
            auto geo = std::make_shared<zeno::GeometryObject>();
            return geo;
        }
    };

    struct ZDEFNODE() Cylinder : INode {
        std::shared_ptr<GeometryObject> apply(
            zeno::vec3f Position,
            zeno::vec3f Scale,
            float radius = 1.f,
            float height = 2.f,
            int lons = 32
        ) {
            auto geo = std::make_shared<zeno::GeometryObject>();
            return geo;
        }
    };
}
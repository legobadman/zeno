#include <zeno/zeno.h>
#include <zeno/core/reflectdef.h>
#include <zeno/types/GeometryObject.h>
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
            },
            _Group {
                {"", ParamObject("Output")},
            }
        };

        std::shared_ptr<GeometryObject> apply(
            zeno::vec3f Center = zeno::vec3f({0,0,0}),
            zeno::vec3f Size = zeno::vec3f({1,1,1}),
            zeno::vec3f Rotate = zeno::vec3f({0,0,0}),
            int x_division = 2,
            int y_division = 2,
            int z_division = 2,
            float uniform_scale = 1.f,
            std::string face_type = "Quadrilaterals"
            )
        {
            if (x_division < 2 || y_division < 2 || z_division < 2) {
                throw makeError<UnimplError>("the division should be greater than 2");
            }

            bool bQuad = face_type == "Quadrilaterals";
            float sizeX = Size[0] * uniform_scale, sizeY = Size[1] * uniform_scale, sizeZ = Size[2] * uniform_scale;
            float xstep = sizeX / (x_division - 1), ystep = sizeY / (y_division - 1), zstep = sizeZ / (z_division - 1);
            float xleft = Center[0] - sizeX / 2, xright = xleft + sizeX;
            float ybottom = Center[1] - sizeY / 2, ytop = ybottom + sizeY;
            float zback = Center[2] - sizeZ / 2, zfront = zback + sizeZ;

            float x, y, z = 0;
            float z_prev = zback;

            //TODO: nFaces需要考虑三角面的情况
            int nPoints = 2 * (x_division * y_division) + (z_division - 2) * (2 * y_division + 2 * x_division - 4);
            int nFaces = 2 * (x_division - 1) * (y_division - 1) + 2 * (x_division - 1) * (z_division - 1) + 2 * (y_division - 1) * (z_division - 1);

            auto geo = std::make_shared<zeno::GeometryObject>(!bQuad, nPoints, nFaces);
            std::vector<vec3f> points;
            points.resize(nPoints);

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
                                        geo->addface({ leftdown, leftup, rightup});
                                        geo->addface({ rightdown, leftdown, rightup});
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
            geo->create_attr(ATTR_POINT, "pos", points);
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
            },
            _Group {
                {"", ParamObject("Output")},
            }
        };

        std::shared_ptr<GeometryObject> apply(
            zeno::vec3f Center = zeno::vec3f({ 0,0,0 }),
            zeno::vec3f Rotate = zeno::vec3f({ 0,0,0 }),
            zeno::vec2f Size = zeno::vec2f({1,1}),
            int Rows = 2,
            int Columns = 2,
            std::string face_type = "Quadrilaterals",
            std::string Direction = "ZX"
        ) {
            if (Rows < 2 || Columns < 2) {
                throw makeError<UnimplError>("the division should be greater than 2");
            }

            float size1 = Size[0], size2 = Size[1];
            float step1 = size1 / (Rows - 1), step2 = size2 / (Columns - 1);
            float bottom1 = Center[0] - size1 / 2, up1 = bottom1 + size1;
            float bottom2 = Center[1] - size2 / 2, up2 = bottom2 + size2;
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
            std::vector<vec3f> points;
            points.resize(nPoints);

            for (size_t i = 0; i < Rows; i++) {
                for (size_t j = 0; j < Columns; j++) {
                    //zeno::vec3f pt(xleft + xstep * x_div, ybottom + ystep * y_div, zfront - zstep * z_div);
                    vec3f pt;
                    if (dir == Dir_ZX) {
                        pt = vec3f(bottom2 + step2 * j, 0, bottom1 + step1 * i);
                    }
                    else if (dir == Dir_YZ) {
                        pt = vec3f(0, bottom1 + step1 * i, bottom2 + step2 * j);
                    }
                    else {
                        pt = vec3f(bottom1 + step1 * i, bottom2 + step2 * j, 0);
                    }

                    size_t idx = i * Columns + j;
                    points[idx] = pt;
                    geo->initpoint(idx);

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
            geo->create_attr(ATTR_POINT, "pos", points);
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
            bool end_caps = false
            )
        {
            bool bQuad = face_type == "Quadrilaterals";
            int nPoints = Rows * Columns;
            int nFaces = (Rows - 1) * (Columns - 1);    //暂不考虑end_caps
            if (!bQuad) {
                nFaces *= 2;
            }

            auto geo = std::make_shared<zeno::GeometryObject>(!bQuad, nPoints, nFaces);
            std::vector<vec3f> points;
            points.resize(nPoints);

            for (int row = 0; row < Rows; row++)
            {
                float up_y = Height / 2.0f, down_y = -Height / 2.0f;
                for (int col = 0; col < Columns; col++)
                {
                    float rad = 2.0f * M_PI * col / Columns;
                    float up_x = up_radius * cos(rad), up_z = up_radius * sin(rad);
                    float down_x = down_radius * cos(rad), down_z = down_radius * sin(rad);
                    vec3f up_pos(up_x, up_y, up_z);
                    vec3f down_pos(down_x, down_y, down_z);

                    size_t idx = row * Columns + col;
                    vec3f pt;
                    if (Direction == "Y Axis") {
                        pt = (float)row / (Rows - 1) * (down_pos - up_pos) + up_pos;
                    }
                    else {
                        throw;
                    }

                    points[idx] = pt;
                    geo->initpoint(idx);
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
            geo->create_attr(ATTR_POINT, "pos", points);
            return geo;
        }
    };

    struct ZDEFNODE() Sphere : INode {

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
            zeno::vec3f Radius = zeno::vec3f(1.f,1.f,1.f),
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

            std::vector<vec3f> points;
            points.resize(nPoints);

            auto geo = std::make_shared<zeno::GeometryObject>(!bQuad, nPoints, nFaces);

            //先加顶部和底部两个顶点
            vec3f topPos(0, Ry, 0);
            int idx = 0;
            points[idx] = topPos;
            geo->initpoint(idx);

            vec3f bottomPos(0, -Ry, 0);
            idx = 1;    //兼容houdini
            points[idx] = bottomPos;
            geo->initpoint(idx);

            float Rxsqr = Rx * Rx;
            float Rysqr = Ry * Ry;
            float Rzsqr = Rz * Rz;

            auto find_x = [=](float y) {
                return sqrt(Rxsqr - Rxsqr * y * y / Rysqr);
            };
            auto find_z = [=](float y) {
                return sqrt(Rzsqr - Rzsqr * y * y / Rysqr);
            };

            float y_piece = 2 * Ry / Rows;

            for (int row = 1; row < Rows - 1; row++)
            {
                float v = (float)row / (float)Rows;
                float theta = M_PI * v;

                float y_pos = topPos[1] - row * y_piece;
                float Rx_level = find_x(y_pos);
                float Rz_level = find_z(y_pos);
                //col = 0, 从x轴正方向开始转圈圈
                size_t startIdx = 2 + (row - 1) * Columns;
                for (int col = 0; col < Columns; col++)
                {
                    //R_level只是定义了y方向界面在x轴的短轴，长轴仍需z方向的R来决定
                    float rad = 2.0f * M_PI * col / Columns;
                    float x_pos = sqrt((Rx_level * Rx_level) / (1.f + pow(tan(rad) * Rx_level / Rz_level, 2)));
                    if (cos(rad) < 0) {
                        x_pos *= -1;
                    }
                    float z_pos = tan(rad) * x_pos;
                    z_pos *= -1;
                    vec3f pt(x_pos, y_pos, z_pos);

                    float u = (float)col / (float)Columns;
                    float phi = M_PI * 2 * u;
                    float x = sin(theta) * cos(phi);
                    float y = cos(theta);
                    float z = -sin(theta) * sin(phi);
                    //pt = vec3f(x, y, z);

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
            geo->create_attr(ATTR_POINT, "pos", points);
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
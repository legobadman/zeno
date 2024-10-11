#include <zeno/zeno.h>
#include <zeno/core/reflectdef.h>
#include <zeno/types/GeometryObject.h>
#include "zeno_types/reflect/reflection.generated.hpp"


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

            int nPoints = 2 * (x_division * y_division) + (z_division - 2) * (2 * y_division + 2 * x_division - 4);
            int nFaces = 2 * (x_division - 1) * (y_division - 1) + 2 * (x_division - 1) * (z_division - 1) + 2 * (y_division - 1) * (z_division - 1);

            auto geo = std::make_shared<zeno::GeometryObject>(!bQuad, nPoints, nFaces);
            std::vector<vec3f> points;
            points.resize(nPoints);

            for (int z_div = 0; z_div < z_division; z_div++)
            {
                if (z_div == 0) {
                    for (int y_div = 0; y_div < y_division; y_div++) {
                        for (int x_div = 0; x_div < x_division; x_div++) {

                            zeno::vec3f pt(xleft + xstep * x_div, ybottom + ystep * y_div, zfront - zstep * z_div);

                            size_t idx = x_div % x_division + y_div * x_division;
                            points[idx] = pt;
                            geo->initpoint(idx);

                            if (x_div > 0 && y_div > 0) {
                                //current traversal point is rightup.
                                size_t leftdown = (x_div - 1) % x_division + (y_div - 1) * x_division;
                                size_t rightdown = leftdown + 1;
                                size_t leftup = (x_div - 1) % x_division + y_div * x_division;
                                size_t rightup = idx;

                                if (bQuad) {
                                    geo->addface({leftdown, rightdown, rightup, leftup});
                                }
                                else {
                                    geo->addface({ leftdown, rightdown, leftup });
                                    geo->addface({ rightdown, rightup, leftup});
                                }
                            }
                        }
                    }
                }
                else if (z_div == z_division - 1) {
                    for (int y_div = 0; y_div < y_division; y_div++) {
                        for (int x_div = 0; x_div < x_division; x_div++) {
                            zeno::vec3f pt(xleft + xstep * x_div, ybottom + ystep * y_div, zfront - zstep * z_div);
                            int nPrevPoints = x_division * y_division + (z_div - 1) * (2 * y_division + 2 * x_division - 4);
                            size_t idx = nPrevPoints + x_div % x_division + y_div * x_division;
                            points[idx] = pt;
                            geo->initpoint(idx);

                            if (x_div > 0 && y_div > 0) {
                                //current traversal point is rightup.
                                size_t leftdown = nPrevPoints + (x_div - 1) % x_division + (y_div - 1) * x_division;
                                size_t rightdown = leftdown + 1;
                                size_t leftup = nPrevPoints + (x_div - 1) % x_division + y_div * x_division;
                                size_t rightup = idx;

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
            int Rows = 1,
            int Columns = 1,
            std::string face_type = "Quadrilaterals",
            std::string Direction = "ZX"
        ) {
            auto geo = std::make_shared<zeno::GeometryObject>();
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
            std::string Direction = "X Axis",
            std::string face_type = "Quadrilaterals",
            bool end_caps = true
            )
        {
            auto geo = std::make_shared<zeno::GeometryObject>();
            return geo;
        }
    };

    struct ZDEFNODE() Sphere : INode {
        std::shared_ptr<GeometryObject> apply(
            zeno::vec3f Position,
            zeno::vec3f Scale,
            zeno::vec3f Rotate,
            bool HasNormal = false,
            bool HasVertUV = false,
            bool IsFlipFace = false,
            int rows = 1,
            int columns = 1,
            bool quads = true,
            bool SphereRT = false)
        {
            auto geo = std::make_shared<zeno::GeometryObject>();
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
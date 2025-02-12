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
                                int leftdown = nPrevPoints + (x_div - 1) % x_division + (y_div - 1) * x_division;
                                int rightdown = leftdown + 1;
                                int leftup = nPrevPoints + (x_div - 1) % x_division + y_div * x_division;
                                int rightup = idx;

                                if (bFirstFace) {
                                    if (bQuad) {
                                        geo->add_face({ leftdown, rightdown, rightup, leftup });
                                    }
                                    else {
                                        geo->add_face({ leftdown, rightdown, leftup });
                                        geo->add_face({ rightdown, rightup, leftup });
                                    }
                                }
                                else {
                                    if (bQuad) {
                                        geo->add_face({ leftdown, leftup, rightup, rightdown });
                                    }
                                    else {
                                        geo->add_face({ leftdown, leftup, rightup });
                                        geo->add_face({ rightup, rightdown, leftdown });
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
                                        geo->add_face({ leftdown, rightdown, rightup, leftup });
                                    }
                                    else {
                                        geo->add_face({ leftdown, rightdown, leftup });
                                        geo->add_face({ rightdown, rightup, leftup });
                                    }
                                }
                                else {
                                    if (bQuad) {
                                        geo->add_face({ leftdown, leftup, rightup, rightdown });
                                    }
                                    else {
                                        geo->add_face({ leftdown, leftup, rightup });
                                        geo->add_face({ rightdown, leftdown, rightup });
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
                                        geo->add_face({ leftdown, rightdown, rightup, leftup });
                                    }
                                    else {
                                        geo->add_face({ leftdown, rightdown, leftup });
                                        geo->add_face({ rightdown, rightup, leftup });
                                    }
                                }
                                else {
                                    if (bQuad) {
                                        geo->add_face({ leftdown, leftup, rightup, rightdown });
                                    }
                                    else {
                                        geo->add_face({ leftdown, leftup, rightup });
                                        geo->add_face({ rightdown, leftdown, rightup });
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

            geo->create_attr(ATTR_POINT, "pos", points);
            if (bCalcPointNormals)
                geo->create_attr(ATTR_POINT, "nrm", normals);
            return geo;
        }

        CustomUI export_customui() const {
            CustomUI ui = INode::export_customui();
            ui.uistyle.iconResPath = "<svg width=\"26\" height=\"26\" viewBox=\"0 0 26 26\" fill=\"none\" xmlns=\"http://www.w3.org/2000/svg\"><path d=\"M12.7307 0.664335L0.976013 4.90657C0.682147 5.00639 0.486237 5.30584 0.486237 5.60529V20.5779C0.486237 20.9023 0.682147 21.1768 0.976013 21.2766L12.7307 25.5188C12.8041 25.5438 12.9021 25.5688 12.9755 25.5688C13.0245 25.5688 13.049 25.5688 13.098 25.5688H13.1225C13.147 25.5688 13.1715 25.5438 13.1959 25.5438L24.9506 21.3016C25.2445 21.2017 25.4404 20.9023 25.4404 20.6028V5.60529V5.58034C25.4404 5.55538 25.4404 5.50548 25.4404 5.48052V5.45557C25.3914 5.20602 25.22 4.98143 24.9751 4.90657L19.0978 2.78545H19.0733C18.9263 2.73554 18.7304 2.71059 18.559 2.78545L7.09822 6.90292C6.80436 7.00274 6.60845 7.30219 6.60845 7.60164V10.3466C6.60845 10.7708 6.9268 11.0952 7.34311 11.0952C7.75942 11.0952 8.07778 10.7708 8.07778 10.3466V8.12568L18.8039 4.25776L22.5262 5.60529L12.7307 9.14881C12.4368 9.24862 12.2409 9.52312 12.2409 9.84753C12.2409 9.84753 12.2409 9.84753 12.2409 9.87248V23.797L1.95557 20.0538V6.12933L13.2204 2.06178C13.6123 1.91205 13.8082 1.48783 13.6612 1.11351C13.5388 0.714244 13.1225 0.514609 12.7307 0.664335ZM13.7102 10.3716L23.9955 6.65337V20.0289L13.7102 23.7471V10.3716Z\" fill=\"#FFFFFF\"/></svg>";
            return ui;
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
                {"Rows", ParamPrimitive("Rows", 2, Slider, std::vector<int>{1,100,1})},
                {"Columns", ParamPrimitive("Columns", 2, Slider, std::vector<int>{1,100,1})}
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

                    int idx = i * Columns + j;
                    points[idx] = pt;
                    geo->initpoint(idx);
                    if (bCalcPointNormals)
                        normals[idx] = nrm;

                    if (j > 0 && i > 0) {
                        int ij = idx;
                        int ij_1 = idx - 1;
                        int i_1j = (i - 1) * Columns + j;
                        int i_1j_1 = i_1j - 1;

                        if (bQuad) {
                            geo->add_face({ ij, i_1j, i_1j_1, ij_1 });
                        }
                        else {
                            geo->add_face({ ij, i_1j, i_1j_1 });
                            geo->add_face({ ij, i_1j_1, ij_1 });
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

            geo->create_attr(ATTR_POINT, "pos", points);
            if (bCalcPointNormals)
                geo->create_attr(ATTR_POINT, "nrm", normals);
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
            float Height = 2.0f,
            int Rows = 2,
            int Columns = 12,
            std::string Direction = "Y Axis",
            std::string face_type = "Quadrilaterals",
            bool bCalcPointNormals = false,
            bool end_caps = true
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

            if (end_caps) {
                //先把顶部和底部两个面加上
                if (bQuad) {
                    std::vector<int> up_pts, down_pts;
                    for (int col = 0; col < Columns; col++)
                    {
                        int up_idx = col;
                        geo->initpoint(up_idx);
                        up_pts.push_back(up_idx);
                    }
                    for (int col = Columns - 1; col >= 0; col--) {
                        int down_idx = (Rows - 1) * Columns + col;
                        geo->initpoint(down_idx);
                        down_pts.push_back(down_idx);
                    }
                    geo->add_face(up_pts);
                    geo->add_face(down_pts);
                }
                else {
                    geo->initpoint(0);
                    geo->initpoint(1);

                    if (Direction == "Y Axis") {
                        float up_y = Height / 2.0f, down_y = -Height / 2.0f;
                        points[0] = vec3f(0, up_y, 0);
                        points[1] = vec3f(0, down_y, 0);
                    }
                    else if (Direction == "X Axis") {
                        float up_x = Height / 2.0f, down_x = -Height / 2.0f;
                        points[0] = vec3f(up_x, 0, 0);
                        points[1] = vec3f(down_x, 0, 0);
                    }
                    else if (Direction == "Z Axis") {
                        float up_z = Height / 2.0f, down_z = -Height / 2.0f;
                        points[0] = vec3f(0, 0, up_z);
                        points[1] = vec3f(0, 0, down_z);
                    }

                    for (int col = 0; col < Columns; col++)
                    {
                        int idx = col + 2;
                        geo->initpoint(idx);
                        if (col > 0) {
                            geo->add_face({ 0, idx - 1, idx });
                        }
                        if (col == Columns - 1) {
                            geo->add_face({ 0, idx, 2 });
                        }
                    }
                    for (int col = 0; col < Columns; col++)
                    {
                        int idx = (Rows - 1) * Columns + col + 2;
                        geo->initpoint(idx);
                        if (col > 0) {
                            geo->add_face({ 1, idx, idx - 1 });
                        }
                        if (col == Columns - 1) {
                            int last_start = (Rows - 1) * Columns + 2;
                            geo->add_face({ 1, last_start, idx});
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

                    vec3f up_pos;
                    vec3f down_pos;
                    if (Direction == "Y Axis") {
                        const float up_y = Height / 2.0f, down_y = -Height / 2.0f;
                        const float up_x = up_radius * cos_a, up_z = -1 * up_radius * sin_a;
                        const float down_x = down_radius * cos_a, down_z = -1 * down_radius * sin_a;
                        up_pos = vec3f(up_x, up_y, up_z);
                        down_pos = vec3f(down_x, down_y, down_z);
                    }
                    else if (Direction == "X Axis") {
                        const float up_x = Height / 2.0f, down_x = -Height / 2.0f;
                        const float up_y = up_radius * cos_a, up_z = -1 * up_radius * sin_a;
                        const float down_y = down_radius * cos_a, down_z = -1 * down_radius * sin_a;
                        up_pos = vec3f(up_x, up_y, up_z);
                        down_pos = vec3f(down_x, down_y, down_z);
                    }
                    else if (Direction == "Z Axis") {
                        const float up_z = Height / 2.0f, down_z = -Height / 2.0f;
                        const float up_y = up_radius * cos_a, up_x = -1 * up_radius * sin_a;
                        const float down_y = down_radius * cos_a, down_x = -1 * down_radius * sin_a;
                        up_pos = vec3f(up_x, up_y, up_z);
                        down_pos = vec3f(down_x, down_y, down_z);
                    }

                    size_t idx = row * Columns + col;
                    if (!bQuad && end_caps) {
                        idx += 2;       //三角面另外加上中轴线顶部和底部两个点
                    }

                    vec3f pt = (float)row / (Rows - 1) * (down_pos - up_pos) + up_pos;
                    points[idx] = pt;
                    geo->initpoint(idx);
                    if (bCalcPointNormals) {
                        normals[idx] = normalize(vec3f(cos_a, tan_belta * (sin_a * sin_a + cos_a * cos_a), -1 * sin_a));
                    }

                    if (row > 0 && col > 0) {
                        int right_bottom = idx;
                        int left_bottom = right_bottom - 1;
                        int right_top = right_bottom - Columns;
                        int left_top = right_top - 1;

                        if (bQuad) {
                            geo->add_face({ left_top, left_bottom, right_bottom, right_top });
                        }
                        else {
                            geo->add_face({ left_top, left_bottom, right_bottom });
                            geo->add_face({ left_top, right_bottom, right_top });
                        }

                        if (col == Columns - 1) {
                            //连接这一圈最后一个点和起始点
                            right_bottom = idx - Columns + 1;
                            left_bottom = idx;
                            left_top = left_bottom - Columns;
                            right_top = right_bottom - Columns;

                            if (bQuad) {
                                geo->add_face({ left_top, left_bottom, right_bottom, right_top });
                            }
                            else {
                                geo->add_face({ left_top, left_bottom, right_bottom });
                                geo->add_face({ left_top, right_bottom, right_top });
                            }
                        }
                    }
                }
            }

            glm::mat4 scale_matrix = glm::scale(glm::mat4(1.0), glm::vec3(uniform_scale, uniform_scale, uniform_scale));
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

            geo->create_attr(ATTR_POINT, "pos", points);
            if (bCalcPointNormals)
                geo->create_attr(ATTR_POINT, "nrm", normals);
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
            Rotate_Orientaion ori;
            if (Direction == "Y Axis") {
                dir = Y_Axis;
                ori = Orientaion_ZX;
            }
            else if (Direction == "X Axis") {
                dir = X_Axis;
                ori = Orientaion_YZ;
            }
            else if (Direction == "Z Axis") {
                dir = Z_Axis;
                ori = Orientaion_XY;
            }
            else {
                throw;
            }
            //dir = Y_Axis;

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
                    else if (dir == X_Axis) {
                        y = sin(theta) * cos(phi);
                        z = -sin(theta) * sin(phi);
                    }

                    vec3f pt = vec3f(x, y, z);

                    int idx = 2/*顶部底部两个点*/ + (row - 1) * Columns + col;
                    geo->initpoint(idx);
                    points[idx] = pt;
                    if (col > 0) {
                        if (row == 1) {
                            //与顶部顶点构成三角面
                            geo->add_face({ 0, idx - 1, idx });
                            if (col == Columns - 1) {
                                geo->add_face({ 0, idx, startIdx });
                            }
                        }
                        else {
                            int rightdown = idx;
                            int leftdown = rightdown - 1;
                            int rightup = 2/*顶部底部两个点*/ + (row - 2) * Columns + col;
                            int leftup = rightup - 1;
                            if (bQuad) {
                                geo->add_face({ rightdown, rightup, leftup, leftdown });
                            }
                            else {
                                geo->add_face({ rightdown, rightup, leftup });
                                geo->add_face({ rightdown, leftup, leftdown });
                            }

                            if (col == Columns - 1) {
                                leftup = rightup;
                                leftdown = rightdown;
                                rightdown = startIdx;
                                rightup = 2 + (row - 2) * Columns;

                                if (bQuad) {
                                    geo->add_face({ rightdown, rightup, leftup, leftdown });
                                }
                                else {
                                    geo->add_face({ rightdown, rightup, leftup });
                                    geo->add_face({ rightdown, leftup, leftdown });
                                }
                            }

                            if (row == Rows - 2) {
                                //与底部顶点构成三角面
                                geo->add_face({ idx, idx - 1, 1 });
                                if (col == Columns - 1) {
                                    geo->add_face({ 1, startIdx, idx });
                                }
                            }
                        }
                    }
                }
            }

            glm::mat4 scale_matrix = glm::scale(glm::mat4(1.0), glm::vec3(uniform_scale * Radius[0], uniform_scale * Radius[1], uniform_scale * Radius[2]));
            glm::mat4 translate = glm::translate(glm::mat4(1.0), glm::vec3(Center[0], Center[1], Center[2]));
            glm::mat4 rotation = calc_rotate_matrix(Rotate[0], Rotate[1], Rotate[2], ori);
            glm::mat4 transform = translate * rotation * scale_matrix;
            for (size_t i = 0; i < points.size(); i++)
            {
                auto pt = points[i]/*, nrm = normals[i]*/;
                glm::vec4 gp = transform * glm::vec4(pt[0], pt[1], pt[2], 1);
                points[i] = zeno::vec3f(gp.x, gp.y, gp.z);
                //glm::vec4 gnrm = rotation * glm::vec4(nrm[0], nrm[1], nrm[2], 0);
                //normals[i] = zeno::vec3f(gnrm.x, gnrm.y, gnrm.z);
                //todo: normal.
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

    struct ZDEFNODE() Circle : INode {

        enum PlaneDirection {
            Dir_XY,
            Dir_YZ,
            Dir_ZX
        };

        ReflectCustomUI m_uilayout = {
            _Group {
                {"segments", ParamPrimitive("Segments", 32, Lineedit)},
                {"radius", ParamPrimitive("Radius", 1.0, Lineedit)},
                {"arcType", ParamPrimitive("Arc Type", "Closed", Combobox, std::vector<std::string>{"Closed", "Open Arc", "Sliced Arc"})},
                {"direction", ParamPrimitive("Direction", "ZX", Combobox, std::vector<std::string>{"XY", "YZ", "ZX"})}
            },
            _Group {
                {"", ParamObject("Output")},
            }
        };

        std::shared_ptr<GeometryObject> apply(
            zeno::vec3f Center = zeno::vec3f({ 0,0,0 }),
            zeno::vec3f Rotate = zeno::vec3f({ 0,0,0 }),
            float segments = 32,
            float radius = 1.0,
            std::string arcType = "Closed",
            zeno::vec2f arcAngle = zeno::vec2i({ 0, 120 }),
            //std::string face_type = "Quadrilaterals",
            std::string direction = "ZX",
            bool bCalcPointNormals = false
        ) {
            if (segments <= 0) {
                throw makeError<UnimplError>("the segments should be positive");
            }
            if (radius < 0) {
                throw makeError<UnimplError>("the segments should not be negative");
            } 

            PlaneDirection dir;
            Rotate_Orientaion ori;
            if (direction == "ZX") {
                dir = Dir_ZX;
                ori = Orientaion_ZX;
            }
            else if (direction == "YZ") {
                dir = Dir_YZ;
                ori = Orientaion_YZ;
            }
            else if (direction == "XY") {
                dir = Dir_XY;
                ori = Orientaion_XY;
            }
            else {
                throw makeError<UnimplError>("Unknown Direction");
            }

            size_t pointNumber = 0, faceNumber = 0;
            if (arcType == "Closed") {
                if (segments == 1) {
                    pointNumber = 1;
                    faceNumber = 0;
                } else {
                    pointNumber = segments + 1;
                    faceNumber = segments;
                }
            }else if (arcType == "Open Arc") {
                pointNumber = segments + 1;
                faceNumber = 0;
            }else if (arcType == "Sliced Arc") {
                pointNumber = segments + 2;
                faceNumber = segments;
            }

            std::shared_ptr<GeometryObject> spgeo = std::make_shared<GeometryObject>(true, pointNumber, faceNumber);

            std::vector<vec3f> points, normals;
            points.resize(pointNumber);
            if (bCalcPointNormals)
                normals.resize(pointNumber);

            if (arcType == "Closed") {
                if (segments == 1) {
                    spgeo->initpoint(0);
                    spgeo->create_attr(ATTR_POINT, "pos", { Center });
                    return spgeo;
                }

                points[0] = Center;//圆心
                spgeo->initpoint(0);

                for (int i = 1; i < pointNumber; i++)
                {
                    float rad = 2.0 * M_PI * (i - 1) / (pointNumber - 1);
                    vec3f pt, nrm;
                    if (dir == Dir_ZX) {
                        pt = vec3f(cos(rad) * radius, 0, -sin(rad) * radius);
                        nrm = vec3f(0, 1, 0);
                    }
                    else if (dir == Dir_YZ) {
                        pt = vec3f(0, cos(rad) * radius, -sin(rad) * radius);
                        nrm = vec3f(1, 0, 0);
                    }
                    else {
                        pt = vec3f(cos(rad) * radius, -sin(rad) * radius, 0);
                        nrm = vec3f(0, 0, 1);
                    }

                    points[i] = pt;
                    if (bCalcPointNormals)
                        normals[i] = nrm;
                    spgeo->initpoint(i);

                    if (i > 1) {
                        spgeo->add_face({ 0, i - 1, i });
                    }

                    if (segments == 2) {//加一条线
                        spgeo->initLineNextPoint(i);
                    }
                }

                spgeo->add_face({ 0, (int)pointNumber - 1, 1 });
            } else if (arcType == "Open Arc") {
                float startAngle = arcAngle[0];
                float arcRange = arcAngle[1] - startAngle;
                for (int i = 0; i < pointNumber; i++)
                {
                    float rad = glm::radians(startAngle + arcRange * i / (pointNumber - 1));
                    vec3f pt, nrm;
                    if (dir == Dir_ZX) {
                        pt = vec3f(cos(rad) * radius, 0, -sin(rad) * radius);
                        nrm = vec3f(0, 1, 0);
                    }
                    else if (dir == Dir_YZ) {
                        pt = vec3f(0, cos(rad) * radius, -sin(rad) * radius);
                        nrm = vec3f(1, 0, 0);
                    }
                    else {
                        pt = vec3f(cos(rad) * radius, -sin(rad) * radius, 0);
                        nrm = vec3f(0, 0, 1);
                    }
                    points[i] = pt;
                    spgeo->initpoint(i);
                    spgeo->initLineNextPoint(i);
                    if (i == pointNumber - 1) {
                        spgeo->setLineNextPt(i, i);
                    }
                }
                bCalcPointNormals = false;
            }else if (arcType == "Sliced Arc") {
                points[0] = Center;//圆心
                spgeo->initpoint(0);

                float startAngle = arcAngle[0];
                float arcRange = arcAngle[1] - startAngle;
                for (int i = 1; i < pointNumber; i++)
                {
                    float rad = glm::radians(startAngle + arcRange * (i - 1) / (pointNumber - 2));
                    vec3f pt, nrm;
                    if (dir == Dir_ZX) {
                        pt = vec3f(cos(rad) * radius, 0, -sin(rad) * radius);
                        nrm = vec3f(0, 1, 0);
                    }
                    else if (dir == Dir_YZ) {
                        pt = vec3f(0, cos(rad) * radius, -sin(rad) * radius);
                        nrm = vec3f(1, 0, 0);
                    }
                    else {
                        pt = vec3f(cos(rad) * radius, -sin(rad) * radius, 0);
                        nrm = vec3f(0, 0, 1);
                    }

                    points[i] = pt;
                    if (bCalcPointNormals)
                        normals[i] = nrm;
                    spgeo->initpoint(i);

                    if (i > 1) {
                        spgeo->add_face({ 0, i - 1, i });
                    }
                }
            }

            glm::mat4 translate = glm::translate(glm::mat4(1.0), glm::vec3(Center[0], Center[1], Center[2]));
            glm::mat4 rotation = calc_rotate_matrix(Rotate[0], Rotate[1], Rotate[2], ori);
            glm::mat4 transform = translate * rotation;
            for (size_t i = arcType == "Open Arc" ? 0 : 1; i < points.size(); i++)
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

            spgeo->create_attr(ATTR_POINT, "pos", points);
            if (bCalcPointNormals) {
                spgeo->create_attr(ATTR_POINT, "nrm", normals);
            }
            return spgeo;
        }
    };

    struct ZDEFNODE() Line : INode {

        ReflectCustomUI m_uilayout = {
            _Group {
            },
            _Group {
                {"", ParamObject("Output")},
            }
        };

        std::shared_ptr<GeometryObject> apply(
            int npoints = 2,
            zeno::vec3f direction = zeno::vec3f({ 0,1,0 }),
            zeno::vec3f origin = zeno::vec3f({ 0,0,0 }),
            float length = 1.0,
            bool isCentered = false,
            bool hasLines = true
        ) {
            if (npoints <= 0) {
                throw makeError<UnimplError>("the npoints should be positive");
            }
            if (length < 0) {
                throw makeError<UnimplError>("the scale should be positive");
            }
            if (direction == zeno::vec3f({0,0,0})) {
                throw makeError<UnimplError>("the direction should not be {0,0,0}");
            }

            std::shared_ptr<GeometryObject> spgeo = std::make_shared<GeometryObject>(true, npoints, 0);

            if (npoints == 1) {
                spgeo->create_attr(ATTR_POINT, "pos", { origin });
                return spgeo;
            }

            float scale = length / glm::sqrt(glm::pow(direction[0], 2) + glm::pow(direction[1], 2) + glm::pow(direction[2], 2)) / (npoints - 1);
            zeno::vec3f ax = direction * scale;
            if (isCentered) {
                origin -= (ax * (npoints - 1)) / 2;
            }

            std::vector<vec3f> points;
            points.resize(npoints);
//#pragma omp parallel for
            for (intptr_t pt = 0; pt < npoints; pt++) {
                vec3f p = origin + pt * ax;
                points[pt] = p;
                spgeo->initpoint(pt);
                if (hasLines) {
                    spgeo->initLineNextPoint(pt);
                    if (pt == npoints - 1) {
                        spgeo->setLineNextPt(pt, pt);
                    }
                }
            }

            spgeo->create_attr(ATTR_POINT, "pos", points);
            return spgeo;
        }
    };
}
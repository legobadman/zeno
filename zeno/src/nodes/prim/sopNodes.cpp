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

    struct ZDEFNODE() CopyToPoints : INode {
        ReflectCustomUI m_uilayout = {
            _Group{
                {"input_object", ParamObject("Input Geometry")},
                {"target_Obj", ParamObject("Target Geometry")},
            },
            _Group{
                {"", ParamObject("Output")},
            }
        };

        std::shared_ptr<GeometryObject> apply(
            std::shared_ptr<zeno::GeometryObject> input_object,
            std::shared_ptr<zeno::GeometryObject> target_Obj
        ) {
            if (!input_object) {
                throw makeError<UnimplError>("empty input object.");
            }
            else if (!target_Obj) {
                throw makeError<UnimplError>("empty target object.");
            }
            if (!input_object->has_point_attr("pos")) {
                throw makeError<UnimplError>("invalid input object.");
            }
            else if (!target_Obj->has_point_attr("pos")) {
                throw makeError<UnimplError>("invalid target object.");
            }

            std::vector<zeno::vec3f> inputPos = input_object->get_attrs<zeno::vec3f>(ATTR_POINT, "pos");
            std::vector<std::vector<int>> inputFacesPoints(input_object->nfaces(), std::vector<int>());
            for (int i = 0; i < input_object->nfaces(); ++i) {
                inputFacesPoints[i] = input_object->face_points(i);
            }


            std::vector<zeno::vec3f> inputNrm;
            std::vector<zeno::vec2f> inputLines;
            bool hasNrm = input_object->has_point_attr("nrm");
            bool isLine = input_object->is_Line();
            if (hasNrm) {
                inputNrm = input_object->get_attrs<zeno::vec3f>(ATTR_POINT, "nrm");
            }
            if (isLine) {
                inputLines.resize(input_object->npoints());
                for (size_t i = 0; i < input_object->npoints(); ++i) {
                    inputLines[i][0] = i;
                    inputLines[i][1] = input_object->getLineNextPt(i);
                }
            }

            zeno::vec3f originCenter, _min, _max;
            std::tie(_min, _max) = geomBoundingBox(input_object.get());
            originCenter = (_min + _max) / 2;

            size_t targetObjPointsCount = target_Obj->npoints();
            size_t inputObjPointsCount = input_object->npoints();
            size_t inputObjFacesCount = input_object->nfaces();
            size_t newObjPointsCount = targetObjPointsCount * inputObjPointsCount;

            std::vector<zeno::vec3f> newObjPos(newObjPointsCount, zeno::vec3f());
            std::vector<zeno::vec3f> newObjNrm;
            if (hasNrm) {
                newObjNrm.resize(newObjPointsCount);
            }

            std::shared_ptr<GeometryObject> spgeo = std::make_shared<GeometryObject>(input_object->is_base_triangle(), newObjPointsCount, 0);
            for (size_t i = 0; i < targetObjPointsCount; ++i) {
                zeno::vec3f targetPos = target_Obj->get_elem<zeno::vec3f>(ATTR_POINT, "pos", 0, i);
                zeno::vec3f dx = targetPos - originCenter;
                glm::mat4 translate = glm::translate(glm::mat4(1.0), glm::vec3(dx[0], dx[1], dx[2]));

                size_t offset = i * inputObjPointsCount;
                for (size_t j = 0; j < inputObjPointsCount; j++)
                {
                    auto idx = offset + j;

                    auto& pt = inputPos[j];
                    glm::vec4 gp = translate * glm::vec4(pt[0], pt[1], pt[2], 1);
                    newObjPos[idx] = zeno::vec3f(gp.x, gp.y, gp.z);

                    if (hasNrm) {
                        newObjNrm[idx] = inputNrm[j];
                    }

                    spgeo->initpoint(idx);

                    if (isLine) {
                        spgeo->initLineNextPoint(idx);
                        if (j == inputObjPointsCount - 1) {
                            spgeo->setLineNextPt(idx, idx);
                        }
                    }
                }
                for (size_t j = 0; j < inputObjFacesCount; ++j)
                {
                    std::vector<int> facePoints = inputFacesPoints[j];
                    for (int k = 0; k < facePoints.size(); ++k) {
                        facePoints[k] += offset;
                    }
                    spgeo->add_face(facePoints);
                }
            }


            spgeo->create_attr(ATTR_POINT, "pos", newObjPos);
            if (hasNrm) {
                spgeo->create_attr(ATTR_POINT, "nrm", newObjNrm);
            }

            return spgeo;
        }
    };

    struct ZDEFNODE() Sweep : INode {

        ReflectCustomUI m_uilayout = {
            _Group{
                {"snapDistance", ParamPrimitive("Snap Distance")},
                {"surface_shape", ParamPrimitive("Surface Shape", "Square Tube", Combobox, std::vector<std::string>{"Second Input", "Square Tube"})},
                {"surface_width", ParamPrimitive("Width", 0.2, Slider, std::vector<float>{0.0, 1.0, 0.001}, "", "visible = parameter('Surface Shape').value == 'Square Tube';")},
                {"surface_column", ParamPrimitive("Columns", 2, Slider, std::vector<int>{1, 20, 1}, "", "visible = parameter('Surface Shape').value == 'Square Tube';")},
                {"input_object", ParamObject("Input")}
            },
            _Group{
                {"", ParamObject("Output")},
            }
        };

        std::shared_ptr<GeometryObject> apply(
            std::shared_ptr<zeno::GeometryObject> input_object,
            std::string surface_shape = "Square Tube",
            float surface_width = 0.2f,
            int surface_column = 2
        )
        {
            const float w = surface_width;
            if (input_object->is_Line()) {
                std::vector<vec3f> pts = input_object->points_pos();
                for (int i = 0; i < pts.size(); i++) {
                    if (i > 0 && i < pts.size() - 1) {
                        vec3f pt = pts[i];
                        vec3f pt_1 = pts[i - 1];
                        vec3f pt1 = pts[i + 1];
                        vec3f v1 = zeno::normalize(pt1 - pt);
                        vec3f v2 = zeno::normalize(pt_1 - pt);
                        while (zeno::dot(v1, v2) == -1) {
                            v1 += vec3f(0.0001, 0, 0);
                            v2 += vec3f(0.0001, 0, 0);
                        }

                        vec3f hor_v = zeno::normalize(v1 + v2);
                        vec3f ver_v = zeno::normalize(zeno::cross(v1, hor_v));

                        //这个start_pt留到最后加
                        vec3f start_pt = pt + w/2.f * (hor_v + ver_v);
                        //逆时针转一圈
                        vec3f curr_start = start_pt;
                        vec3f curr_pt;
                        for (int i = 1; i <= surface_column; i++) {
                            curr_pt = curr_start + w / surface_column * i * (-hor_v);
                            int j;
                            j = 0;
                        }
                        curr_start = curr_pt; //上一次遍历的终点是下一个线段的起点
                        for (int i = 1; i <= surface_column; i++) {
                            curr_pt = curr_start + w / surface_column * i * (-ver_v);
                            int j;
                            j = 0;
                        }
                        curr_start = curr_pt;
                        for (int i = 1; i <= surface_column; i++) {
                            curr_pt = curr_start + w / surface_column * i * hor_v;
                            int j;
                            j = 0;
                        }
                        curr_start = curr_pt;
                        for (int i = 1; i <= surface_column; i++) {
                            curr_pt = curr_start + w / surface_column * i * ver_v;
                            int j;
                            j = 0;
                        }
                    }
                }
            }
            return input_object;
        }
    };

    struct ZDEFNODE() AverageFuse : INode {
        //houdini fuse节点的average模式

        ReflectCustomUI m_uilayout = {
            _Group{
                {"snapDistance", ParamPrimitive("Snap Distance")},
                {"input_object", ParamObject("Input Geometry")},
            },
            _Group{
                {"", ParamObject("Output")},
            }
        };

        std::shared_ptr<GeometryObject> apply(
            std::shared_ptr<zeno::GeometryObject> input_object,
            float snapDistance = 0.01
        ) {
            if (!input_object) {
                throw makeError<UnimplError>("empty input object.");
            }

            int ptnumber = input_object->npoints();
            std::map < int, std::vector<int> > pointsToFuse;
            std::vector<int> fusedPoints(ptnumber, -1);
            const std::vector<zeno::vec3f>& inputPos = input_object->get_attrs<zeno::vec3f>(ATTR_POINT, "pos");
            for (int i = 0; i < inputPos.size(); ++i) {
                if (fusedPoints[i] != -1) {
                    continue;
                }
                for (int j = i + 1; j < inputPos.size(); ++j) {
                    if (fusedPoints[j] != -1) {
                        continue;
                    }
                    if (glm::distance(glm::vec3(inputPos[i][0], inputPos[i][1], inputPos[i][2]), glm::vec3(inputPos[j][0], inputPos[j][1], inputPos[j][2])) <= snapDistance) {
                        pointsToFuse[i].push_back(j);
                        fusedPoints[j] = i;
                    }
                }
            }

            for (auto& [targetPt, fusePoints] : pointsToFuse) {
                zeno::vec3f pos = input_object->get_elem<zeno::vec3f>(ATTR_POINT, "pos", 0, targetPt);
                for (auto& pt : fusePoints) {
                    pos += input_object->get_elem<zeno::vec3f>(ATTR_POINT, "pos", 0, pt);
                }
                pos /= (fusePoints.size() + 1);
                input_object->set_elem<zeno::vec3f>(ATTR_POINT, "pos", targetPt, pos);
            }
            input_object->fusePoints(fusedPoints);

            return input_object;
        }
    };
}

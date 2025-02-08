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

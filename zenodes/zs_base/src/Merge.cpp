#include <zvec.h>
#include <vec.h>
#include <inodedata.h>
#include <inodeimpl.h>
#include "api.h"
#include "zcommon.h"
#include "typehelper.h"
#include "utils.h"
#include <Windows.h>
#include <vector>
#include <string>
#include <glm/glm.hpp>

namespace zeno {

    static const char* kCommonPointAttrs[] = { "nrm", "uv", "clr", "cd" };
    static const int kNumCommonAttrs = sizeof(kCommonPointAttrs) / sizeof(kCommonPointAttrs[0]);

    struct Merge : INode2 {

        zeno::NodeType type() const override { return zeno::Node_Normal; }
        float time() const override { return 1.f; }
        void clearCalcResults() override {}

        ZErrorCode apply(zeno::INodeData* ptrNodeData) override
        {
            auto list_object = ptrNodeData->get_input_ListObject("Input Of Objects");
            if (!list_object) {
                ptrNodeData->report_error("empty input list");
                return ZErr_ParamError;
            }
            if (list_object->empty()) {
                ptrNodeData->report_error("input list is empty");
                return ZErr_ParamError;
            }

            std::vector<IGeometryObject*> geoms;
            size_t nItems = list_object->size();
            for (size_t i = 0; i < nItems; i++) {
                IObject2* obj = list_object->get((int)i);
                if (!obj) continue;
                if (obj->type() != ZObj_Geometry) continue;
                auto geom = static_cast<IGeometryObject*>(obj);
                if (!geom->has_point_attr("pos")) continue;
                if (geom->npoints() == 0 && geom->nfaces() == 0) continue;
                geoms.push_back(geom);
            }
            if (geoms.empty()) {
                ptrNodeData->report_error("no valid geometry in list");
                return ZErr_ParamError;
            }

            size_t totalPoints = 0;
            size_t totalFaces = 0;
            for (auto g : geoms) {
                totalPoints += (size_t)g->npoints();
                totalFaces += (size_t)g->nfaces();
            }
            if (totalPoints == 0) {
                ptrNodeData->report_error("merged geometry has no points");
                return ZErr_ParamError;
            }

            std::vector<zeno::vec3f> mergedPos(totalPoints);
            std::vector<std::vector<int>> mergedFaces;
            mergedFaces.reserve(totalFaces);

            size_t ptOffset = 0;
            size_t faceOffset = 0;
            for (auto geom : geoms) {
                int nPts = geom->npoints();
                int nFaces = geom->nfaces();

                std::vector<zeno::Vec3f> posBuf(nPts);
                size_t nr = geom->get_vec3f_attr(ATTR_POINT, "pos", posBuf.data(), nPts);
                if (nr != (size_t)nPts) continue;
                for (int i = 0; i < nPts; i++) {
                    mergedPos[ptOffset + i] = zeno::vec3f(posBuf[i].x, posBuf[i].y, posBuf[i].z);
                }
                for (int i = 0; i < nFaces; i++) {
                    int nv = geom->face_vertex_count(i);
                    std::vector<int> pts(nv);
                    size_t got = geom->face_points(i, pts.data(), nv);
                    if (got != (size_t)nv) continue;
                    for (int k = 0; k < nv; k++) pts[k] += (int)ptOffset;
                    mergedFaces.push_back(std::move(pts));
                }
                ptOffset += nPts;
            }

            auto spOutput = zeno::create_GeometryObject(Topo_IndiceMesh2, false, mergedPos, mergedFaces);
            if (!spOutput) {
                ptrNodeData->report_error("failed to create merged geometry");
                return ZErr_UnimplError;
            }

            for (int a = 0; a < kNumCommonAttrs; a++) {
                const char* attrName = kCommonPointAttrs[a];
                bool anyVec3 = false;
                bool anyFloat = false;
                for (auto g : geoms) {
                    if (g->has_attr(ATTR_POINT, attrName, ATTR_VEC3)) anyVec3 = true;
                    if (g->has_attr(ATTR_POINT, attrName, ATTR_FLOAT)) anyFloat = true;
                }
                if (!anyVec3 && !anyFloat) continue;
                bool isVec3 = anyVec3;

                if (isVec3) {
                    std::vector<zeno::Vec3f> mergedVec(totalPoints);
                    glm::vec3 defVal(0.f);
                    ptOffset = 0;
                    for (auto geom : geoms) {
                        int nPts = geom->npoints();
                        if (geom->has_attr(ATTR_POINT, attrName, ATTR_VEC3)) {
                            std::vector<zeno::Vec3f> buf(nPts);
                            geom->get_vec3f_attr(ATTR_POINT, attrName, buf.data(), nPts);
                            for (int i = 0; i < nPts; i++) {
                                mergedVec[ptOffset + i] = buf[i];
                            }
                        } else {
                            for (int i = 0; i < nPts; i++) {
                                mergedVec[ptOffset + i] = zeno::Vec3f{ defVal.x, defVal.y, defVal.z };
                            }
                        }
                        ptOffset += nPts;
                    }
                    spOutput->create_attr_by_vec3(ATTR_POINT, attrName, mergedVec.data(), totalPoints);
                } else {
                    std::vector<float> mergedFloat(totalPoints);
                    float defVal = 0.f;
                    ptOffset = 0;
                    for (auto geom : geoms) {
                        int nPts = geom->npoints();
                        if (geom->has_attr(ATTR_POINT, attrName, ATTR_FLOAT)) {
                            std::vector<float> buf(nPts);
                            geom->get_float_attr(ATTR_POINT, attrName, buf.data(), nPts);
                            for (int i = 0; i < nPts; i++) {
                                mergedFloat[ptOffset + i] = buf[i];
                            }
                        } else {
                            for (int i = 0; i < nPts; i++) {
                                mergedFloat[ptOffset + i] = defVal;
                            }
                        }
                        ptOffset += nPts;
                    }
                    spOutput->create_attr_by_float(ATTR_POINT, attrName, mergedFloat.data(), totalPoints);
                }
            }

            ptrNodeData->set_output_object("Output", spOutput);
            return ZErr_OK;
        }
    };

    ZENDEFNODE_ABI(Merge,
        Z_INPUTS(
            { "Input Of Objects", _gParamType_List }
        ),
        Z_OUTPUTS(
            { "Output", _gParamType_Geometry }
        ),
        "prim",
        "",
        "",
        ""
    );
}

#include <vec.h>
#include <glm/glm.hpp>
#include <inodedata.h>
#include <inodeimpl.h>
#include "api.h"
#include "zcommon.h"
#include "typehelper.h"
#include "utils.h"
#include <Windows.h>
#include <vector>
#include <algorithm>

namespace zeno {

    static glm::vec3 mirrorPointAcrossPlane(
        const glm::vec3& p,
        const glm::vec3& planePoint,
        const glm::vec3& planeNormalUnit)
    {
        float signedDist = glm::dot(p - planePoint, planeNormalUnit);
        return p - 2.0f * signedDist * planeNormalUnit;
    }

    struct Mirror : INode2 {

        zeno::NodeType type() const override { return zeno::Node_Normal; }
        float time() const override { return 1.f; }
        void clearCalcResults() override {}

        ZErrorCode apply(zeno::INodeData* ptrNodeData) override
        {
            auto input_geom = ptrNodeData->get_input_Geometry("Input Object");
            if (!input_geom) {
                ptrNodeData->report_error("empty input object");
                return ZErr_ParamError;
            }
            if (!input_geom->has_point_attr("pos")) {
                ptrNodeData->report_error("input has no pos attr");
                return ZErr_ParamError;
            }

            Vec3f originAbi = ptrNodeData->get_input2_vec3f("Origin");
            Vec3f directionAbi = ptrNodeData->get_input2_vec3f("Direction");
            float distance = ptrNodeData->get_input2_float("Distance");
            bool keepOriginal = ptrNodeData->get_input2_bool("Keep Original");

            glm::vec3 origin(originAbi.x, originAbi.y, originAbi.z);
            glm::vec3 direction(directionAbi.x, directionAbi.y, directionAbi.z);
            float nLen = glm::length(direction);
            if (nLen <= 1e-8f) {
                ptrNodeData->report_error("Direction must be non-zero");
                return ZErr_ParamError;
            }
            glm::vec3 n = direction / nLen;
            glm::vec3 planePoint = origin + n * distance;

            int nPts = input_geom->npoints();
            int nFaces = input_geom->nfaces();
            if (nPts == 0) {
                ptrNodeData->set_output_object("Output", input_geom->clone());
                return ZErr_OK;
            }

            std::vector<Vec3f> posAbi((size_t)nPts);
            size_t got = input_geom->get_vec3f_attr(ATTR_POINT, "pos", posAbi.data(), nPts);
            if (got != (size_t)nPts) {
                ptrNodeData->report_error("failed to read point positions");
                return ZErr_ParamError;
            }

            std::vector<glm::vec3> srcPos((size_t)nPts);
            for (int i = 0; i < nPts; ++i) {
                srcPos[(size_t)i] = glm::vec3(posAbi[(size_t)i].x, posAbi[(size_t)i].y, posAbi[(size_t)i].z);
            }

            bool hasNrm = input_geom->has_point_attr("nrm");
            std::vector<glm::vec3> srcNrm;
            if (hasNrm) {
                std::vector<Vec3f> nrmAbi((size_t)nPts);
                size_t ngot = input_geom->get_vec3f_attr(ATTR_POINT, "nrm", nrmAbi.data(), nPts);
                if (ngot == (size_t)nPts) {
                    srcNrm.resize((size_t)nPts);
                    for (int i = 0; i < nPts; ++i) {
                        srcNrm[(size_t)i] = glm::vec3(nrmAbi[(size_t)i].x, nrmAbi[(size_t)i].y, nrmAbi[(size_t)i].z);
                    }
                }
                else {
                    hasNrm = false;
                }
            }

            std::vector<glm::vec3> outPos;
            outPos.resize((size_t)(keepOriginal ? (2 * nPts) : nPts));

            std::vector<glm::vec3> outNrm;
            if (hasNrm) outNrm.resize(outPos.size());

            if (keepOriginal) {
                for (int i = 0; i < nPts; ++i) {
                    outPos[(size_t)i] = srcPos[(size_t)i];
                    outPos[(size_t)(i + nPts)] = mirrorPointAcrossPlane(srcPos[(size_t)i], planePoint, n);

                    if (hasNrm) {
                        outNrm[(size_t)i] = srcNrm[(size_t)i];
                        outNrm[(size_t)(i + nPts)] = mirrorPointAcrossPlane(srcNrm[(size_t)i], glm::vec3(0.0f), n);
                    }
                }
            }
            else {
                for (int i = 0; i < nPts; ++i) {
                    outPos[(size_t)i] = mirrorPointAcrossPlane(srcPos[(size_t)i], planePoint, n);
                    if (hasNrm) {
                        outNrm[(size_t)i] = mirrorPointAcrossPlane(srcNrm[(size_t)i], glm::vec3(0.0f), n);
                    }
                }
            }

            std::vector<std::vector<int>> outFaces;
            outFaces.reserve((size_t)(keepOriginal ? (2 * nFaces) : nFaces));

            if (keepOriginal) {
                for (int f = 0; f < nFaces; ++f) {
                    int nv = input_geom->face_vertex_count(f);
                    if (nv < 3) continue;
                    std::vector<int> face((size_t)nv);
                    size_t fg = input_geom->face_points(f, face.data(), nv);
                    if (fg == (size_t)nv) outFaces.push_back(face);
                }
            }

            for (int f = 0; f < nFaces; ++f) {
                int nv = input_geom->face_vertex_count(f);
                if (nv < 3) continue;
                std::vector<int> face((size_t)nv);
                size_t fg = input_geom->face_points(f, face.data(), nv);
                if (fg != (size_t)nv) continue;

                // Reflection flips winding; reverse to keep outward orientation.
                std::reverse(face.begin(), face.end());
                if (keepOriginal) {
                    for (int& idx : face) idx += nPts;
                }
                outFaces.push_back(std::move(face));
            }

            auto output = create_GeometryObject(Topo_IndiceMesh2, false, outPos, outFaces);
            if (!output) {
                ptrNodeData->report_error("failed to create output geometry");
                return ZErr_UnimplError;
            }

            if (hasNrm) {
                std::vector<Vec3f> outNrmAbi(outNrm.size());
                for (size_t i = 0; i < outNrm.size(); ++i) {
                    outNrmAbi[i] = Vec3f{ outNrm[i].x, outNrm[i].y, outNrm[i].z };
                }
                output->create_attr_by_vec3(ATTR_POINT, "nrm", outNrmAbi.data(), (int)outNrmAbi.size());
            }

            ptrNodeData->set_output_object("Output", output);
            return ZErr_OK;
        }
    };

    ZENDEFNODE_ABI(Mirror,
        Z_INPUTS(
            { "Input Object", _gParamType_Geometry },
            { "Origin", _gParamType_Vec3f, ZVec3f(0, 0, 0) },
            { "Direction", _gParamType_Vec3f, ZVec3f(1, 0, 0) },
            { "Distance", _gParamType_Float, ZFloat(0.0f), Lineedit },
            { "Keep Original", _gParamType_Bool, ZInt(1), Checkbox }
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


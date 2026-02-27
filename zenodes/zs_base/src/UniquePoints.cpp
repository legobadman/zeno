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

namespace zeno {

    struct UniquePoints : INode2 {
        zeno::NodeType type() const override { return zeno::Node_Normal; }
        float time() const override { return 1.f; }
        void clearCalcResults() override {}

        ZErrorCode apply(zeno::INodeData* ptrNodeData) override {
            auto input_geom = ptrNodeData->get_input_Geometry("Input");
            bool postComputeNormals = ptrNodeData->get_input2_bool("Post-Compute Normals");
            if (!input_geom) {
                ptrNodeData->report_error("empty input object");
                return ZErr_ParamError;
            }
            if (!input_geom->has_point_attr("pos")) {
                ptrNodeData->report_error("input has no pos attr");
                return ZErr_ParamError;
            }

            int nPts = input_geom->npoints();
            int nFaces = input_geom->nfaces();
            std::vector<Vec3f> posAbi((size_t)nPts);
            size_t gotPos = input_geom->get_vec3f_attr(ATTR_POINT, "pos", posAbi.data(), nPts);
            if (gotPos != (size_t)nPts) {
                ptrNodeData->report_error("failed to read input positions");
                return ZErr_ParamError;
            }
            std::vector<glm::vec3> srcPos((size_t)nPts);
            for (int i = 0; i < nPts; ++i) {
                srcPos[(size_t)i] = glm::vec3(posAbi[(size_t)i].x, posAbi[(size_t)i].y, posAbi[(size_t)i].z);
            }

            std::vector<glm::vec3> outPos;
            std::vector<std::vector<int>> outFaces;
            std::vector<glm::vec3> outNrm;

            outFaces.reserve((size_t)nFaces);
            for (int f = 0; f < nFaces; ++f) {
                int nv = input_geom->face_vertex_count(f);
                if (nv < 3) continue;
                std::vector<int> face((size_t)nv);
                size_t gotFace = input_geom->face_points(f, face.data(), nv);
                if (gotFace != (size_t)nv) continue;

                std::vector<int> newFace;
                newFace.reserve((size_t)nv);
                for (int idx : face) {
                    if (idx < 0 || idx >= nPts) continue;
                    newFace.push_back((int)outPos.size());
                    outPos.push_back(srcPos[(size_t)idx]);
                }
                if (newFace.size() >= 3) {
                    if (postComputeNormals) {
                        glm::vec3 n(0.0f);
                        if (newFace.size() > 2) {
                            glm::vec3 p0 = outPos[(size_t)newFace[0]];
                            glm::vec3 p1 = outPos[(size_t)newFace[1]];
                            glm::vec3 p2 = outPos[(size_t)newFace[2]];
                            n = glm::cross(p1 - p0, p2 - p1);
                            float l = glm::length(n);
                            if (l > 1e-8f) n /= l;
                        }
                        for (size_t i = 0; i < newFace.size(); ++i) outNrm.push_back(n);
                    }
                    outFaces.push_back(std::move(newFace));
                }
            }

            auto output = create_GeometryObject(Topo_IndiceMesh2, false, outPos, outFaces);
            if (!output) {
                ptrNodeData->report_error("failed to create output geometry");
                return ZErr_UnimplError;
            }
            if (postComputeNormals && outNrm.size() == outPos.size()) {
                std::vector<Vec3f> nrmAbi(outNrm.size());
                for (size_t i = 0; i < outNrm.size(); ++i) {
                    nrmAbi[i] = Vec3f{ outNrm[i].x, outNrm[i].y, outNrm[i].z };
                }
                output->create_attr_by_vec3(ATTR_POINT, "nrm", nrmAbi.data(), (int)nrmAbi.size());
            }

            ptrNodeData->set_output_object("Output", output);
            return ZErr_OK;
        }
    };

    ZENDEFNODE_ABI(UniquePoints,
        Z_INPUTS(
            { "Input", _gParamType_Geometry },
            { "Post-Compute Normals", _gParamType_Bool, ZInt(0), Checkbox }
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


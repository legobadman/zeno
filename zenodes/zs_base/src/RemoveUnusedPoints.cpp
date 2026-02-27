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

    struct RemoveUnusedPoints : INode2 {
        zeno::NodeType type() const override { return zeno::Node_Normal; }
        float time() const override { return 1.f; }
        void clearCalcResults() override {}

        ZErrorCode apply(zeno::INodeData* ptrNodeData) override {
            auto input_geom = ptrNodeData->get_input_Geometry("Input Object");
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
            std::vector<glm::vec3> pos((size_t)nPts);
            for (int i = 0; i < nPts; ++i) {
                pos[(size_t)i] = glm::vec3(posAbi[(size_t)i].x, posAbi[(size_t)i].y, posAbi[(size_t)i].z);
            }

            bool hasNrm = input_geom->has_point_attr("nrm");
            std::vector<glm::vec3> nrm;
            if (hasNrm) {
                std::vector<Vec3f> nrmAbi((size_t)nPts);
                size_t gotNrm = input_geom->get_vec3f_attr(ATTR_POINT, "nrm", nrmAbi.data(), nPts);
                if (gotNrm == (size_t)nPts) {
                    nrm.resize((size_t)nPts);
                    for (int i = 0; i < nPts; ++i) {
                        nrm[(size_t)i] = glm::vec3(nrmAbi[(size_t)i].x, nrmAbi[(size_t)i].y, nrmAbi[(size_t)i].z);
                    }
                }
                else {
                    hasNrm = false;
                }
            }

            std::vector<std::vector<int>> faces;
            faces.reserve((size_t)nFaces);
            std::vector<int> used((size_t)nPts, 0);

            for (int f = 0; f < nFaces; ++f) {
                int nv = input_geom->face_vertex_count(f);
                if (nv < 3) continue;
                std::vector<int> face((size_t)nv);
                size_t gotFace = input_geom->face_points(f, face.data(), nv);
                if (gotFace != (size_t)nv) continue;
                for (int idx : face) {
                    if (idx >= 0 && idx < nPts) used[(size_t)idx] = 1;
                }
                faces.push_back(std::move(face));
            }

            std::vector<int> remap((size_t)nPts, -1);
            std::vector<glm::vec3> compactPos;
            compactPos.reserve((size_t)nPts);
            std::vector<glm::vec3> compactNrm;
            if (hasNrm) compactNrm.reserve((size_t)nPts);

            for (int i = 0; i < nPts; ++i) {
                if (!used[(size_t)i]) continue;
                remap[(size_t)i] = (int)compactPos.size();
                compactPos.push_back(pos[(size_t)i]);
                if (hasNrm) compactNrm.push_back(nrm[(size_t)i]);
            }

            for (auto& face : faces) {
                for (int& idx : face) {
                    idx = (idx >= 0 && idx < nPts) ? remap[(size_t)idx] : -1;
                }
            }

            auto output = create_GeometryObject(Topo_IndiceMesh2, false, compactPos, faces);
            if (!output) {
                ptrNodeData->report_error("failed to create output geometry");
                return ZErr_UnimplError;
            }
            if (hasNrm && compactNrm.size() == compactPos.size()) {
                std::vector<Vec3f> nrmAbi(compactNrm.size());
                for (size_t i = 0; i < compactNrm.size(); ++i) {
                    nrmAbi[i] = Vec3f{ compactNrm[i].x, compactNrm[i].y, compactNrm[i].z };
                }
                output->create_attr_by_vec3(ATTR_POINT, "nrm", nrmAbi.data(), (int)nrmAbi.size());
            }

            ptrNodeData->set_output_object("Output", output);
            return ZErr_OK;
        }
    };

    ZENDEFNODE_ABI(RemoveUnusedPoints,
        Z_INPUTS(
            { "Input Object", _gParamType_Geometry }
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


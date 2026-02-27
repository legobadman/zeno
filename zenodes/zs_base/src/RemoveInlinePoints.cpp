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
#include <cmath>

namespace zeno {

    struct RemoveInlinePoints : INode2 {
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
            std::vector<int> faceUseCount((size_t)nPts, 0);

            for (int f = 0; f < nFaces; ++f) {
                int nv = input_geom->face_vertex_count(f);
                if (nv < 3) continue;
                std::vector<int> face((size_t)nv);
                size_t gotFace = input_geom->face_points(f, face.data(), nv);
                if (gotFace != (size_t)nv) continue;
                for (int idx : face) {
                    if (idx >= 0 && idx < nPts) faceUseCount[(size_t)idx]++;
                }
                faces.push_back(std::move(face));
            }

            const float eps = 1e-6f;
            for (auto& face : faces) {
                bool changed = true;
                while (changed && face.size() > 2) {
                    changed = false;
                    int nv = (int)face.size();
                    for (int i = 0; i < nv; ++i) {
                        int ip = (i - 1 + nv) % nv;
                        int in = (i + 1) % nv;
                        int v = face[(size_t)i];
                        if (v < 0 || v >= nPts) continue;
                        if (faceUseCount[(size_t)v] != 1) continue; // same as old node: only point owned by one face

                        glm::vec3 pa = pos[(size_t)face[(size_t)ip]];
                        glm::vec3 pb = pos[(size_t)v];
                        glm::vec3 pc = pos[(size_t)face[(size_t)in]];
                        glm::vec3 d1 = pb - pa;
                        glm::vec3 d2 = pc - pa;
                        float l1 = glm::length(d1);
                        float l2 = glm::length(d2);
                        if (l1 <= eps || l2 <= eps) continue;
                        d1 /= l1;
                        d2 /= l2;
                        float c = glm::dot(d1, d2);
                        if (std::abs(c - 1.0f) <= 1e-6f) {
                            face.erase(face.begin() + i);
                            faceUseCount[(size_t)v] = 0;
                            changed = true;
                            break;
                        }
                    }
                }
            }

            std::vector<int> remap((size_t)nPts, -1);
            std::vector<glm::vec3> compactPos;
            compactPos.reserve((size_t)nPts);
            std::vector<glm::vec3> compactNrm;
            if (hasNrm) compactNrm.reserve((size_t)nPts);

            std::vector<std::vector<int>> outFaces;
            outFaces.reserve(faces.size());
            for (auto& face : faces) {
                if (face.size() < 3) continue;
                std::vector<int> outFace;
                outFace.reserve(face.size());
                for (int idx : face) {
                    if (idx < 0 || idx >= nPts) continue;
                    if (remap[(size_t)idx] < 0) {
                        remap[(size_t)idx] = (int)compactPos.size();
                        compactPos.push_back(pos[(size_t)idx]);
                        if (hasNrm) compactNrm.push_back(nrm[(size_t)idx]);
                    }
                    outFace.push_back(remap[(size_t)idx]);
                }
                if (outFace.size() >= 3) outFaces.push_back(std::move(outFace));
            }

            auto output = create_GeometryObject(Topo_IndiceMesh2, false, compactPos, outFaces);
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

    ZENDEFNODE_ABI(RemoveInlinePoints,
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


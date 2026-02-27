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
#include <string>
#include <cmath>

namespace zeno {

    struct Clip : INode2 {

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
            std::string Keep = get_string_param(ptrNodeData, "Keep");
            Vec3f centerAbi = ptrNodeData->get_input2_vec3f("Center Position");
            Vec3f dirAbi = ptrNodeData->get_input2_vec3f("Direction");

            glm::vec3 center(centerAbi.x, centerAbi.y, centerAbi.z);
            glm::vec3 dir(dirAbi.x, dirAbi.y, dirAbi.z);
            float dirLen = glm::length(dir);
            if (dirLen <= 1e-8f) {
                ptrNodeData->report_error("Direction must be non-zero");
                return ZErr_ParamError;
            }
            glm::vec3 n = dir / dirLen;

            enum KeepMode { KeepBoth, KeepPositive, KeepNegative };
            KeepMode mode = KeepBoth;
            if (Keep == "All") {
                mode = KeepBoth;
            }
            else if (Keep == "Part Below The Plane") {
                // Keep legacy sopNodes.cpp mapping:
                // "Below" -> Keep_Above in old divideObject.
                mode = KeepPositive;
            }
            else if (Keep == "Part Above The Plane") {
                // Keep legacy sopNodes.cpp mapping:
                // "Above" -> Keep_Below in old divideObject.
                mode = KeepNegative;
            }
            else {
                ptrNodeData->report_error("invalid Keep option");
                return ZErr_ParamError;
            }

            int nPts = input_geom->npoints();
            int nFaces = input_geom->nfaces();
            if (nPts == 0 || nFaces == 0) {
                ptrNodeData->set_output_object("Output", input_geom->clone());
                return ZErr_OK;
            }

            std::vector<Vec3f> posAbi((size_t)nPts);
            size_t gotPos = input_geom->get_vec3f_attr(ATTR_POINT, "pos", posAbi.data(), nPts);
            if (gotPos != (size_t)nPts) {
                ptrNodeData->report_error("failed to read positions");
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
                size_t gotNrm = input_geom->get_vec3f_attr(ATTR_POINT, "nrm", nrmAbi.data(), nPts);
                if (gotNrm == (size_t)nPts) {
                    srcNrm.resize((size_t)nPts);
                    for (int i = 0; i < nPts; ++i) {
                        srcNrm[(size_t)i] = glm::vec3(nrmAbi[(size_t)i].x, nrmAbi[(size_t)i].y, nrmAbi[(size_t)i].z);
                    }
                }
                else {
                    hasNrm = false;
                }
            }

            std::vector<glm::vec3> outPos = srcPos;
            std::vector<glm::vec3> outNrm;
            if (hasNrm) outNrm = srcNrm;
            std::vector<std::vector<int>> outFaces;
            outFaces.reserve((size_t)nFaces * (mode == KeepBoth ? 2 : 1));

            const float eps = 1e-6f;
            auto signedDist = [&](int idx) {
                return glm::dot(srcPos[(size_t)idx] - center, n);
            };

            auto isInside = [&](float d, bool keepPos) {
                return keepPos ? (d >= -eps) : (d <= eps);
            };

            auto addIntersection = [&](int ia, int ib, float da, float db) -> int {
                float denom = da - db;
                float t = 0.5f;
                if (std::abs(denom) > 1e-12f) {
                    t = da / denom;
                }
                t = std::min(std::max(t, 0.0f), 1.0f);

                glm::vec3 p = srcPos[(size_t)ia] + (srcPos[(size_t)ib] - srcPos[(size_t)ia]) * t;
                int idx = (int)outPos.size();
                outPos.push_back(p);
                if (hasNrm) {
                    glm::vec3 nn = srcNrm[(size_t)ia] + (srcNrm[(size_t)ib] - srcNrm[(size_t)ia]) * t;
                    float ll = glm::length(nn);
                    if (ll > 1e-8f) nn /= ll;
                    outNrm.push_back(nn);
                }
                return idx;
            };

            auto clipFaceOneSide = [&](const std::vector<int>& face, bool keepPos) {
                std::vector<int> poly;
                int nv = (int)face.size();
                if (nv < 3) return poly;
                poly.reserve((size_t)nv + 2);
                for (int i = 0; i < nv; ++i) {
                    int ia = face[(size_t)i];
                    int ib = face[(size_t)((i + 1) % nv)];
                    float da = signedDist(ia);
                    float db = signedDist(ib);
                    bool ina = isInside(da, keepPos);
                    bool inb = isInside(db, keepPos);

                    if (ina && inb) {
                        poly.push_back(ib);
                    }
                    else if (ina && !inb) {
                        poly.push_back(addIntersection(ia, ib, da, db));
                    }
                    else if (!ina && inb) {
                        poly.push_back(addIntersection(ia, ib, da, db));
                        poly.push_back(ib);
                    }
                }
                // remove consecutive duplicates
                std::vector<int> cleaned;
                cleaned.reserve(poly.size());
                for (int idx : poly) {
                    if (cleaned.empty() || cleaned.back() != idx) cleaned.push_back(idx);
                }
                if (cleaned.size() >= 2 && cleaned.front() == cleaned.back()) cleaned.pop_back();
                if (cleaned.size() < 3) cleaned.clear();
                return cleaned;
            };

            for (int f = 0; f < nFaces; ++f) {
                int nv = input_geom->face_vertex_count(f);
                if (nv < 3) continue;
                std::vector<int> face((size_t)nv);
                size_t gotFace = input_geom->face_points(f, face.data(), nv);
                if (gotFace != (size_t)nv) continue;

                if (mode == KeepBoth || mode == KeepPositive) {
                    auto p = clipFaceOneSide(face, true);
                    if (!p.empty()) outFaces.push_back(std::move(p));
                }
                if (mode == KeepBoth || mode == KeepNegative) {
                    auto p = clipFaceOneSide(face, false);
                    if (!p.empty()) outFaces.push_back(std::move(p));
                }
            }

            if (outFaces.empty()) {
                ptrNodeData->set_output_object("Output", input_geom->clone());
                return ZErr_OK;
            }

            // Compact points to referenced subset.
            std::vector<int> remap(outPos.size(), -1);
            std::vector<glm::vec3> compactPos;
            std::vector<glm::vec3> compactNrm;
            compactPos.reserve(outPos.size());
            if (hasNrm) compactNrm.reserve(outNrm.size());

            for (auto& face : outFaces) {
                for (int& idx : face) {
                    if (idx < 0 || (size_t)idx >= outPos.size()) continue;
                    if (remap[(size_t)idx] < 0) {
                        remap[(size_t)idx] = (int)compactPos.size();
                        compactPos.push_back(outPos[(size_t)idx]);
                        if (hasNrm) compactNrm.push_back(outNrm[(size_t)idx]);
                    }
                    idx = remap[(size_t)idx];
                }
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

    ZENDEFNODE_ABI(Clip,
        Z_INPUTS(
            { "Input Object", _gParamType_Geometry },
            { "Keep", _gParamType_String, ZString("All"), Combobox, Z_STRING_ARRAY("All", "Part Below The Plane", "Part Above The Plane") },
            { "Center Position", _gParamType_Vec3f, ZVec3f(0,0,0) },
            { "Direction", _gParamType_Vec3f, ZVec3f(0,1,0) }
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

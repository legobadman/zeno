#include <vec.h>
#include <glm/glm.hpp>
#include <inodedata.h>
#include <inodeimpl.h>
#include <Windows.h>
#include "api.h"
#include "zcommon.h"
#include "vecutil.h"
#include "typehelper.h"
#include "utils.h"
#include <vector>
#include <unordered_map>
#include <cmath>

namespace zeno {

    static glm::vec3 safe_normalize(const glm::vec3& v) {
        float len2 = glm::dot(v, v);
        if (len2 <= 1e-12f) {
            return glm::vec3(0.0f, 0.0f, 0.0f);
        }
        return v / glm::sqrt(len2);
    }

    struct Extrude : INode2 {
        zeno::NodeType type() const override { return zeno::Node_Normal; }
        float time() const override { return 1.f; }
        void clearCalcResults() override {}

        ZErrorCode apply(INodeData* ptrNodeData) override {
            auto input_geom = ptrNodeData->get_input_Geometry("Input");
            float distance = ptrNodeData->get_input2_float("Distance");
            float inset = ptrNodeData->get_input2_float("Inset");
            bool output_front = ptrNodeData->get_input2_bool("Output Front");
            bool output_back = ptrNodeData->get_input2_bool("Output Back");
            bool output_side = ptrNodeData->get_input2_bool("Output Side");

            if (!input_geom) {
                ptrNodeData->report_error("empty input geometry");
                return ZErr_ParamError;
            }
            if (!input_geom->has_point_attr("pos")) {
                ptrNodeData->report_error("input has no pos attr");
                return ZErr_ParamError;
            }

            int nPts = input_geom->npoints();
            int nFaces = input_geom->nfaces();
            if (nPts == 0 || nFaces == 0) {
                ptrNodeData->set_output_object("Output", input_geom->clone());
                return ZErr_OK;
            }

            std::vector<Vec3f> posAbi(nPts);
            size_t gotPos = input_geom->get_vec3f_attr(ATTR_POINT, "pos", posAbi.data(), nPts);
            if (gotPos != (size_t)nPts) {
                ptrNodeData->report_error("failed to read point positions");
                return ZErr_ParamError;
            }

            // Original point positions.
            std::vector<glm::vec3> basePos(nPts);
            for (int i = 0; i < nPts; ++i) {
                basePos[i] = glm::vec3(posAbi[i].x, posAbi[i].y, posAbi[i].z);
            }

            // 1. Compute per-face normals and accumulate to points.
            std::vector<glm::vec3> accumNormal(nPts, glm::vec3(0.0f));

            for (int f = 0; f < nFaces; ++f) {
                int nv = input_geom->face_vertex_count(f);
                if (nv < 3) continue;

                std::vector<int> vidx(nv);
                size_t got = input_geom->face_points(f, vidx.data(), nv);
                if (got != (size_t)nv) continue;

                const glm::vec3& p0 = basePos[vidx[0]];
                const glm::vec3& p1 = basePos[vidx[1]];
                const glm::vec3& p2 = basePos[vidx[2]];
                glm::vec3 fn = glm::normalize(glm::cross(p1 - p0, p2 - p0));

                if (glm::dot(fn, fn) <= 1e-12f) continue;

                for (int i = 0; i < nv; ++i) {
                    int v = vidx[i];
                    if (v >= 0 && v < nPts) {
                        accumNormal[v] += fn;
                    }
                }
            }

            std::vector<glm::vec3> pointNormal(nPts);
            for (int i = 0; i < nPts; ++i) {
                pointNormal[i] = safe_normalize(accumNormal[i]);
            }

            // 2. Build output point list: bottom (original) + top (extruded, optional inset).
            std::vector<glm::vec3> outPos;
            outPos.reserve((size_t)nPts * 2);
            for (int i = 0; i < nPts; ++i) {
                outPos.push_back(basePos[i]);
            }

            // Per-vertex extruded index.
            std::vector<int> extrudedIndex(nPts, -1);
            // Per-face top point indices: face -> [pt0, pt1, ...].
            std::vector<std::vector<int>> faceTopPoints(nFaces);

            const bool useInset = (std::abs(inset) > 1e-6f);

            if (!useInset) {
                // No inset: one top point per vertex, along point normal.
                for (int i = 0; i < nPts; ++i) {
                    glm::vec3 q = basePos[i] + pointNormal[i] * distance;
                    extrudedIndex[i] = (int)outPos.size();
                    outPos.push_back(q);
                }
            }
            else {
                // Inset: compute face-driven target positions, then average to shared per-point tops.
                std::vector<glm::vec3> insetAccum(nPts, glm::vec3(0.0f));
                std::vector<int> insetCount(nPts, 0);

                for (int f = 0; f < nFaces; ++f) {
                    int nv = input_geom->face_vertex_count(f);
                    if (nv < 3) continue;

                    std::vector<int> vidx(nv);
                    size_t got = input_geom->face_points(f, vidx.data(), nv);
                    if (got != (size_t)nv) continue;

                    // Face normal and extruded face center.
                    const glm::vec3& p0 = basePos[vidx[0]];
                    const glm::vec3& p1 = basePos[vidx[1]];
                    const glm::vec3& p2 = basePos[vidx[2]];
                    glm::vec3 faceN = safe_normalize(glm::cross(p1 - p0, p2 - p0));
                    glm::vec3 faceCenter(0.0f);
                    for (int i = 0; i < nv; ++i) {
                        faceCenter += basePos[vidx[i]];
                    }
                    faceCenter /= (float)nv;
                    glm::vec3 faceCenterExt = faceCenter + faceN * distance;

                    for (int i = 0; i < nv; ++i) {
                        int v = vidx[i];
                        if (v < 0 || v >= nPts) continue;
                        glm::vec3 pExt = basePos[v] + pointNormal[v] * distance;
                        glm::vec3 toCenter = faceCenterExt - pExt;
                        float len = glm::length(toCenter);
                        if (len > 1e-8f) {
                            pExt += (toCenter / len) * inset;
                        }
                        insetAccum[v] += pExt;
                        insetCount[v] += 1;
                    }
                }

                for (int i = 0; i < nPts; ++i) {
                    glm::vec3 q = basePos[i] + pointNormal[i] * distance;
                    if (insetCount[i] > 0) {
                        q = insetAccum[i] / (float)insetCount[i];
                    }
                    extrudedIndex[i] = (int)outPos.size();
                    outPos.push_back(q);
                }

                for (int f = 0; f < nFaces; ++f) {
                    int nv = input_geom->face_vertex_count(f);
                    if (nv < 3) continue;
                    std::vector<int> vidx(nv);
                    size_t got = input_geom->face_points(f, vidx.data(), nv);
                    if (got != (size_t)nv) continue;

                    faceTopPoints[f].resize((size_t)nv);
                    for (int i = 0; i < nv; ++i) {
                        int v = vidx[i];
                        faceTopPoints[f][i] = (v >= 0 && v < nPts) ? extrudedIndex[v] : v;
                    }
                }
            }

            // 3. Copy original faces (bottom) and create top faces.
            std::vector<std::vector<int>> outFaces;
            outFaces.reserve((size_t)nFaces * 3);

            for (int f = 0; f < nFaces; ++f) {
                int nv = input_geom->face_vertex_count(f);
                if (nv < 3) continue;

                std::vector<int> vidx(nv);
                size_t got = input_geom->face_points(f, vidx.data(), nv);
                if (got != (size_t)nv) continue;

                if (output_back) {
                    outFaces.push_back(vidx);
                }
                if (output_front) {
                    if (useInset && !faceTopPoints[f].empty()) {
                        outFaces.push_back(faceTopPoints[f]);
                    }
                    else {
                        std::vector<int> top(nv);
                        for (int i = 0; i < nv; ++i) {
                            int v = vidx[i];
                            top[i] = (v >= 0 && v < nPts) ? extrudedIndex[v] : v;
                        }
                        outFaces.push_back(std::move(top));
                    }
                }
            }

            // 4. Build side faces for boundary edges.
            struct EdgeKey {
                int a, b;
            };
            struct EdgeInfo {
                int count = 0;
                int a = -1, b = -1;
                int face = -1;  // one of the faces (for boundary: the only face)
            };

            auto makeKey = [](int u, int v) {
                EdgeKey k;
                if (u < v) { k.a = u; k.b = v; }
                else      { k.a = v; k.b = u; }
                return k;
            };

            auto edgeHash = [](const EdgeKey& k) {
                return std::hash<int>()(k.a * 73856093 ^ k.b * 19349663);
            };

            auto edgeEq = [](const EdgeKey& lhs, const EdgeKey& rhs) {
                return lhs.a == rhs.a && lhs.b == rhs.b;
            };

            std::unordered_map<EdgeKey, EdgeInfo, decltype(edgeHash), decltype(edgeEq)> edgeMap(0, edgeHash, edgeEq);

            for (int f = 0; f < nFaces; ++f) {
                int nv = input_geom->face_vertex_count(f);
                if (nv < 3) continue;

                std::vector<int> vidx(nv);
                size_t got = input_geom->face_points(f, vidx.data(), nv);
                if (got != (size_t)nv) continue;

                for (int i = 0; i < nv; ++i) {
                    int v0 = vidx[i];
                    int v1 = vidx[(i + 1) % nv];
                    if (v0 == v1) continue;

                    EdgeKey key = makeKey(v0, v1);
                    auto& info = edgeMap[key];
                    info.count += 1;
                    info.a = v0;
                    info.b = v1;
                    info.face = f;
                }
            }

            for (const auto& kv : edgeMap) {
                const EdgeInfo& info = kv.second;
                if (info.count != 1) continue;
                int v0 = info.a;
                int v1 = info.b;
                if (v0 < 0 || v1 < 0 || v0 >= nPts || v1 >= nPts) continue;

                int top0 = extrudedIndex[v0];
                int top1 = extrudedIndex[v1];
                if (top0 < 0 || top1 < 0) continue;
                if (output_side) {
                    outFaces.push_back({ v0, v1, top1, top0 });
                }
            }

            auto output = create_GeometryObject(Topo_IndiceMesh2, false, outPos, outFaces);
            if (!output) {
                ptrNodeData->report_error("failed to create output geometry");
                return ZErr_UnimplError;
            }

            ptrNodeData->set_output_object("Output", output);
            return ZErr_OK;
        }
    };

    ZENDEFNODE_ABI(Extrude,
        Z_INPUTS(
            { "Input", _gParamType_Geometry },
            { "Distance", _gParamType_Float, ZFloat(0.1f), Lineedit },
            { "Inset", _gParamType_Float, ZFloat(0.0f), Lineedit },
            { "Output Front", _gParamType_Bool, ZInt(1), Checkbox },
            { "Output Back", _gParamType_Bool, ZInt(1), Checkbox },
            { "Output Side", _gParamType_Bool, ZInt(1), Checkbox }
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

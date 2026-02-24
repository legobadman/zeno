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
#include <map>
#include <set>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <cmath>

namespace zeno {

    // Apply Angle.x (u-direction slant) and Angle.y (v-direction slant) in degrees.
    // Angle.x: tilt of divisions along v (lines of constant u become slanted).
    // Angle.y: tilt of divisions along u (lines of constant v become slanted).
    static glm::vec2 brickerAngle(glm::vec2 uv, float angleXDeg, float angleYDeg, glm::vec2 center) {
        float ax = angleXDeg * glm::pi<float>() / 180.f;
        float ay = angleYDeg * glm::pi<float>() / 180.f;
        glm::vec2 d = uv - center;
        float shearX = std::tan(ax);
        float shearY = std::tan(ay);
        return center + glm::vec2(d.x + shearX * d.y, d.y + shearY * d.x);
    }

    struct Divide : INode2 {

        zeno::NodeType type() const override { return zeno::Node_Normal; }
        float time() const override { return 1.f; }
        void clearCalcResults() override {}

        ZErrorCode apply(zeno::INodeData* ptrNodeData) override
        {
            auto input_geom = ptrNodeData->get_input_Geometry("Input");
            zeno::Vec3f sizeAbi = ptrNodeData->get_input2_vec3f("Size");
            zeno::Vec3f offsetAbi = ptrNodeData->get_input2_vec3f("Offset");
            zeno::Vec3f angleAbi = ptrNodeData->get_input2_vec3f("Angle");
            bool brickerSharedEdges = ptrNodeData->get_input2_bool("Bricker Shared Edges");
            bool removeSharedEdges = ptrNodeData->get_input2_bool("Remove Shared Edges");

            glm::vec2 size(std::max(1.f, sizeAbi.x), std::max(1.f, sizeAbi.y));
            glm::vec2 offset(offsetAbi.x, offsetAbi.y);
            int divU = (int)size.x;
            int divV = (int)size.y;

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

            std::vector<zeno::Vec3f> posAbi(nPts);
            input_geom->get_vec3f_attr(ATTR_POINT, "pos", posAbi.data(), nPts);
            std::vector<glm::vec3> pos(nPts);
            for (int i = 0; i < nPts; i++)
                pos[i] = glm::vec3(posAbi[i].x, posAbi[i].y, posAbi[i].z);

            using EdgeKey = std::pair<int, int>;
            auto makeEdge = [](int a, int b) -> EdgeKey {
                return a < b ? EdgeKey(a, b) : EdgeKey(b, a);
            };

            std::map<EdgeKey, std::vector<glm::vec3>> edgePoints;

            auto sampleEdge = [&](int pa, int pb, int nSeg) -> std::vector<glm::vec3> {
                if (nSeg <= 0) return {};
                EdgeKey ek = makeEdge(pa, pb);
                if (brickerSharedEdges) {
                    auto it = edgePoints.find(ek);
                    if (it != edgePoints.end()) return it->second;
                }
                std::vector<glm::vec3> pts;
                pts.reserve(nSeg + 1);
                for (int k = 0; k <= nSeg; k++) {
                    float t = (float)k / (float)nSeg;
                    pts.push_back(glm::mix(pos[pa], pos[pb], t));
                }
                if (brickerSharedEdges) edgePoints[ek] = pts;
                return pts;
            };

            std::vector<glm::vec3> outPos;
            std::vector<std::vector<int>> outFaces;

            for (int fi = 0; fi < nFaces; fi++) {
                int nv = input_geom->face_vertex_count(fi);
                std::vector<int> fpts(nv);
                size_t got = input_geom->face_points(fi, fpts.data(), nv);
                if (got != (size_t)nv) continue;

                if (nv == 3) {
                    glm::vec3 v0 = pos[fpts[0]], v1 = pos[fpts[1]], v2 = pos[fpts[2]];
                    for (int j = 0; j < divU; j++) {
                        for (int k = 0; k < divV; k++) {
                            float s0 = (j + offset.x) / divU, t0 = (k + offset.y) / divV;
                            float s1 = (j + 1 + offset.x) / divU, t1 = (k + 1 + offset.y) / divV;
                            glm::vec2 c(0.5f, 0.5f);
                            glm::vec2 p0 = brickerAngle(glm::vec2(s0, t0), angleAbi.x, angleAbi.y, c);
                            glm::vec2 p1 = brickerAngle(glm::vec2(s1, t0), angleAbi.x, angleAbi.y, c);
                            glm::vec2 p2 = brickerAngle(glm::vec2(s1, t1), angleAbi.x, angleAbi.y, c);
                            glm::vec2 p3 = brickerAngle(glm::vec2(s0, t1), angleAbi.x, angleAbi.y, c);
                            auto bary = [&](glm::vec2 uv) {
                                float u = glm::clamp(uv.x, 0.f, 1.f);
                                float v = glm::clamp(uv.y, 0.f, 1.f);
                                if (u + v > 1.f) { u = 1.f - u; v = 1.f - v; }
                                return v0 * (1.f - u - v) + v1 * u + v2 * v;
                            };
                            int i0 = (int)outPos.size();
                            outPos.push_back(bary(p0));
                            outPos.push_back(bary(p1));
                            outPos.push_back(bary(p2));
                            outPos.push_back(bary(p3));
                            outFaces.push_back({ i0, i0 + 1, i0 + 2, i0 + 3 });
                        }
                    }
                } else if (nv >= 4) {
                    auto bilinear = [&](float s, float t) {
                        s = glm::clamp(s, 0.f, 1.f);
                        t = glm::clamp(t, 0.f, 1.f);
                        return (1.f - s) * (1.f - t) * pos[fpts[0]]
                            + s * (1.f - t) * pos[fpts[1]]
                            + s * t * pos[fpts[2]]
                            + (1.f - s) * t * pos[fpts[3]];
                    };
                    for (int j = 0; j < divU; j++) {
                        for (int k = 0; k < divV; k++) {
                            float s0 = (j + offset.x) / divU, t0 = (k + offset.y) / divV;
                            float s1 = (j + 1 + offset.x) / divU, t1 = (k + 1 + offset.y) / divV;
                            glm::vec2 c(0.5f, 0.5f);
                            glm::vec2 p0 = brickerAngle(glm::vec2(s0, t0), angleAbi.x, angleAbi.y, c);
                            glm::vec2 p1 = brickerAngle(glm::vec2(s1, t0), angleAbi.x, angleAbi.y, c);
                            glm::vec2 p2 = brickerAngle(glm::vec2(s1, t1), angleAbi.x, angleAbi.y, c);
                            glm::vec2 p3 = brickerAngle(glm::vec2(s0, t1), angleAbi.x, angleAbi.y, c);
                            int i0 = (int)outPos.size();
                            outPos.push_back(bilinear(p0.x, p0.y));
                            outPos.push_back(bilinear(p1.x, p1.y));
                            outPos.push_back(bilinear(p2.x, p2.y));
                            outPos.push_back(bilinear(p3.x, p3.y));
                            outFaces.push_back({ i0, i0 + 1, i0 + 2, i0 + 3 });
                        }
                    }
                }
            }

            if (removeSharedEdges && !outFaces.empty()) {
                auto idxOf = [](const std::vector<int>& f, int v) {
                    for (size_t i = 0; i < f.size(); i++) if (f[i] == v) return (int)i;
                    return -1;
                };
                for (;;) {
                    std::map<std::pair<int, int>, std::vector<int>> edgeToFaces;
                    for (size_t fi = 0; fi < outFaces.size(); fi++) {
                        const auto& f = outFaces[fi];
                        for (size_t i = 0; i < f.size(); i++) {
                            int a = f[i], b = f[(i + 1) % f.size()];
                            auto ek = a < b ? std::make_pair(a, b) : std::make_pair(b, a);
                            edgeToFaces[ek].push_back((int)fi);
                        }
                    }
                    bool changed = false;
                    for (const auto& [edge, faces] : edgeToFaces) {
                        if (faces.size() != 2) continue;
                        int f0 = faces[0], f1 = faces[1];
                        const auto& fa = outFaces[f0], fb = outFaces[f1];
                        int a = edge.first, b = edge.second;

                        int ia = idxOf(fa, a), ib = idxOf(fa, b);
                        int ja = idxOf(fb, a), jb = idxOf(fb, b);
                        if (ia < 0 || ib < 0 || ja < 0 || jb < 0) continue;

                        bool fa_a_then_b = (fa[(ia + 1) % fa.size()] == b);
                        bool fb_a_then_b = (fb[(ja + 1) % fb.size()] == b);
                        int nfb = (int)fb.size();

                        auto collectMiddle = [&](int start, int step) {
                            std::vector<int> mid;
                            for (int k = 0; k < nfb - 2; k++) {
                                mid.push_back(fb[start]);
                                start = (start + step + nfb) % nfb;
                            }
                            return mid;
                        };
                        std::vector<int> middle;
                        if (fa_a_then_b) {
                            if (fb_a_then_b) middle = collectMiddle((ja - 1 + nfb) % nfb, -1);
                            else middle = collectMiddle((ja + 1) % nfb, 1);
                        } else {
                            if (fb_a_then_b) middle = collectMiddle((jb + 1) % nfb, 1);
                            else middle = collectMiddle((jb - 1 + nfb) % nfb, -1);
                        }

                        std::vector<int> merged;
                        if (fa_a_then_b) {
                            for (int i = 0; i <= ia; i++) merged.push_back(fa[i]);
                            for (int v : middle) merged.push_back(v);
                            for (int i = ia + 2; i < (int)fa.size(); i++) merged.push_back(fa[i]);
                        } else {
                            for (int i = 0; i <= ib; i++) merged.push_back(fa[i]);
                            for (int v : middle) merged.push_back(v);
                            for (int i = ib + 2; i < (int)fa.size(); i++) merged.push_back(fa[i]);
                        }

                        int keep = std::min(f0, f1), drop = std::max(f0, f1);
                        outFaces[keep] = std::move(merged);
                        outFaces.erase(outFaces.begin() + drop);
                        changed = true;
                        break;
                    }
                    if (!changed) break;
                }
            }

            if (outPos.empty()) {
                ptrNodeData->set_output_object("Output", input_geom->clone());
                return ZErr_OK;
            }

            std::vector<glm::vec3> posForApi(outPos.size());
            for (size_t i = 0; i < outPos.size(); i++)
                posForApi[i] = outPos[i];

            auto spOutput = zeno::create_GeometryObject(Topo_IndiceMesh2, false, posForApi, outFaces);
            if (!spOutput) {
                ptrNodeData->report_error("failed to create output geometry");
                return ZErr_UnimplError;
            }
            ptrNodeData->set_output_object("Output", spOutput);
            return ZErr_OK;
        }
    };

    ZENDEFNODE_ABI(Divide,
        Z_INPUTS(
            { "Input", _gParamType_Geometry },
            { "Size", _gParamType_Vec3f, ZVec3f(2, 2, 0) },
            { "Offset", _gParamType_Vec3f, ZVec3f(0, 0, 0) },
            { "Angle", _gParamType_Vec3f, ZVec3f(0, 0, 0) },
            { "Bricker Shared Edges", _gParamType_Bool, ZInt(1) },
            { "Remove Shared Edges", _gParamType_Bool, ZInt(0) }
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

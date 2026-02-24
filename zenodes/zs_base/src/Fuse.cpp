#include <zvec.h>
#include <vec.h>
#include <inodedata.h>
#include <inodeimpl.h>
#include <Windows.h>
#include "api.h"
#include "zcommon.h"
#include "typehelper.h"
#include "utils.h"
#include <vector>
#include <map>
#include <glm/glm.hpp>
#include <cmath>

namespace zeno {

    struct UnionFind {
        std::vector<int> parent;
        UnionFind(int n) : parent(n) {
            for (int i = 0; i < n; i++) parent[i] = i;
        }
        int find(int x) {
            if (parent[x] != x) parent[x] = find(parent[x]);
            return parent[x];
        }
        void unite(int a, int b) {
            a = find(a), b = find(b);
            if (a != b) parent[a] = b;
        }
    };

    struct Fuse : INode2 {

        zeno::NodeType type() const override { return zeno::Node_Normal; }
        float time() const override { return 1.f; }
        void clearCalcResults() override {}

        ZErrorCode apply(zeno::INodeData* ptrNodeData) override
        {
            auto input_geom = ptrNodeData->get_input_Geometry("Input");
            float dist = ptrNodeData->get_input2_float("dist");

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

            float distSq = dist * dist;
            UnionFind uf(nPts);
            for (int i = 0; i < nPts; i++) {
                for (int j = i + 1; j < nPts; j++) {
                    if (glm::dot(pos[i] - pos[j], pos[i] - pos[j]) <= distSq + 1e-9f)
                        uf.unite(i, j);
                }
            }

            std::vector<int> repId(nPts);
            std::map<int, int> groupToNewIdx;
            int newIdx = 0;
            for (int i = 0; i < nPts; i++) {
                int r = uf.find(i);
                if (groupToNewIdx.find(r) == groupToNewIdx.end())
                    groupToNewIdx[r] = newIdx++;
                repId[i] = groupToNewIdx[r];
            }

            int nNewPts = (int)groupToNewIdx.size();
            std::vector<glm::vec3> newPos(nNewPts);
            std::vector<int> count(nNewPts, 0);
            for (int i = 0; i < nPts; i++) {
                int idx = repId[i];
                newPos[idx] += pos[i];
                count[idx]++;
            }
            for (int i = 0; i < nNewPts; i++)
                newPos[i] /= (float)count[i];

            std::vector<std::vector<int>> newFaces;
            for (int fi = 0; fi < nFaces; fi++) {
                int nv = input_geom->face_vertex_count(fi);
                std::vector<int> fpts(nv);
                size_t got = input_geom->face_points(fi, fpts.data(), nv);
                if (got != (size_t)nv) continue;

                std::vector<int> remapped;
                for (int v : fpts)
                    remapped.push_back(repId[v]);
                std::vector<int> cleaned;
                for (size_t i = 0; i < remapped.size(); i++) {
                    int a = remapped[i];
                    int b = remapped[(i + 1) % remapped.size()];
                    if (a != b) cleaned.push_back(a);
                }
                bool allSame = true;
                for (size_t i = 1; i < cleaned.size() && allSame; i++)
                    if (cleaned[i] != cleaned[0]) allSame = false;
                if (cleaned.size() < 3 || allSame) continue;
                newFaces.push_back(std::move(cleaned));
            }

            std::vector<glm::vec3> posForApi(newPos.begin(), newPos.end());
            auto spOutput = zeno::create_GeometryObject(Topo_IndiceMesh2, false, posForApi, newFaces);
            if (!spOutput) {
                ptrNodeData->report_error("failed to create output geometry");
                return ZErr_UnimplError;
            }
            ptrNodeData->set_output_object("Output", spOutput);
            return ZErr_OK;
        }
    };

    ZENDEFNODE_ABI(Fuse,
        Z_INPUTS(
            { "Input", _gParamType_Geometry },
            { "dist", _gParamType_Float, ZFloat(1e-5f) }
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

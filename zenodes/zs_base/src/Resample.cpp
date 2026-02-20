#include <vec.h>
#include <inodedata.h>
#include <inodeimpl.h>
#include "api.h"
#include "zcommon.h"
#include "vecutil.h"
#include "typehelper.h"
#include "utils.h"
#include <Windows.h>
#include <vector>
#include <cmath>

namespace zeno {

    struct Resample : INode2 {

        zeno::NodeType type() const override { return zeno::Node_Normal; }
        float time() const override { return 1.f; }
        void clearCalcResults() override {}

        ZErrorCode apply(zeno::INodeData* ptrNodeData) override
        {
            auto input_geom = ptrNodeData->get_input_Geometry("Input Object");
            float Length = ptrNodeData->get_input2_float("Length");

            if (!input_geom) {
                ptrNodeData->report_error("empty input object");
                return ZErr_ParamError;
            }
            if (Length <= 0.f) {
                ptrNodeData->set_output_object("Output", input_geom->clone());
                return ZErr_OK;
            }

            int npts = input_geom->npoints();
            int nface = input_geom->nfaces();
            std::vector<zeno::Vec3f> posAbi(npts);
            size_t n = input_geom->get_vec3f_attr(ATTR_POINT, "pos", posAbi.data(), npts);
            if (n != (size_t)npts) {
                ptrNodeData->report_error("failed to get point positions");
                return ZErr_ParamError;
            }
            std::vector<zeno::vec3f> pos(npts);
            for (int i = 0; i < npts; i++) pos[i] = toVec3f(posAbi[i]);

            std::vector<zeno::vec3f> newpos;
            std::vector<std::vector<int>> newfaces;
            for (int iFace = 0; iFace < nface; iFace++) {
                int nv = input_geom->face_vertex_count(iFace);
                std::vector<int> face_pts(nv);
                size_t got = input_geom->face_points(iFace, face_pts.data(), nv);
                if (got != (size_t)nv) continue;

                std::vector<int> newface;
                for (int i = 0; i < nv; i++) {
                    int startPt = (i == 0) ? face_pts[nv - 1] : face_pts[i - 1];
                    int endPt = face_pts[i];
                    zeno::vec3f startPos = pos[startPt];
                    zeno::vec3f endPos = pos[endPt];
                    zeno::vec3f dir = endPos - startPos;
                    float L = std::sqrt(dir[0] * dir[0] + dir[1] * dir[1] + dir[2] * dir[2]);
                    if (L < 1e-8f) continue;
                    dir[0] /= L; dir[1] /= L; dir[2] /= L;
                    int k = (int)(L / Length);
                    for (int j = 0; j < k; j++) {
                        zeno::vec3f mid = startPos + (float)j * Length * dir;
                        newface.push_back((int)newpos.size());
                        newpos.push_back(mid);
                    }
                }
                newfaces.push_back(std::move(newface));
            }

            bool bTri = (input_geom->topo_type() == Topo_IndiceMesh || input_geom->topo_type() == Topo_IndiceMesh2);
            auto spOutput = zeno::create_GeometryObject(Topo_IndiceMesh, bTri, newpos, newfaces);
            ptrNodeData->set_output_object("Output", spOutput);
            return ZErr_OK;
        }
    };

    ZENDEFNODE_ABI(Resample,
        Z_INPUTS(
            { "Input Object", _gParamType_Geometry },
            { "Length", _gParamType_Float, ZFloat(0.1f) }
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

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
#include <algorithm>

namespace zeno {

    struct Reverse : INode2 {

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

            int npts = input_geom->npoints();
            int nface = input_geom->nfaces();
            if (npts == 0 || nface == 0) {
                ptrNodeData->set_output_object("Output", input_geom->clone());
                return ZErr_OK;
            }

            std::vector<zeno::Vec3f> posAbi(npts);
            size_t n = input_geom->get_vec3f_attr(ATTR_POINT, "pos", posAbi.data(), npts);
            if (n != (size_t)npts) {
                ptrNodeData->report_error("failed to get point positions");
                return ZErr_ParamError;
            }
            std::vector<zeno::vec3f> pos(npts);
            for (size_t i = 0; i < n; i++) {
                pos[i] = toVec3f(posAbi[i]);
            }

            std::vector<std::vector<int>> faces;
            for (int iFace = 0; iFace < nface; iFace++) {
                int nv = input_geom->face_vertex_count(iFace);
                std::vector<int> pts(nv);
                size_t got = input_geom->face_points(iFace, pts.data(), nv);
                if (got != (size_t)nv) continue;
                std::reverse(pts.begin() + 1, pts.end());
                faces.emplace_back(std::move(pts));
            }

            bool bTri = (input_geom->topo_type() == Topo_IndiceMesh || input_geom->topo_type() == Topo_IndiceMesh2);
            auto spOutput = zeno::create_GeometryObject(Topo_IndiceMesh, bTri, pos, faces);
            ptrNodeData->set_output_object("Output", spOutput);
            return ZErr_OK;
        }
    };

    ZENDEFNODE_ABI(Reverse,
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

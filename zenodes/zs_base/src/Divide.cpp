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

#if 0
// Original implementation depends on divideObject, geomBoundingBox from zeno/geo/geometryutil.h
// which is not exposed in the zenodes API. Include original when API is extended.
/*
#include <zeno/zeno.h>
#include <zeno/types/GeometryObject.h>
#include <zeno/geo/geometryutil.h>

struct Divide : INode {
    void apply() override {
        auto input_object = ZImpl(get_input2<GeometryObject_Adapter>("Input"));
        zeno::vec3f Size = ZImpl(get_input2<zeno::vec3f>("Size"));
        bool remove_shared_edge = ZImpl(get_input2<bool>("Remove Shared Edges"));
        auto bbox = geomBoundingBox(input_object->m_impl.get());
        // ... divideObject calls
    }
};
*/
#endif

namespace zeno {

    struct Divide : INode2 {

        zeno::NodeType type() const override { return zeno::Node_Normal; }
        float time() const override { return 1.f; }
        void clearCalcResults() override {}

        ZErrorCode apply(zeno::INodeData* ptrNodeData) override
        {
#if 0
            auto input_geom = ptrNodeData->get_input_Geometry("Input");
            if (!input_geom) {
                ptrNodeData->report_error("empty input object");
                return ZErr_ParamError;
            }
            zeno::Vec3f sizeAbi = ptrNodeData->get_input2_vec3f("Size");
            zeno::vec3f Size = toVec3f(sizeAbi);
            bool remove_shared_edge = ptrNodeData->get_input2_bool("Remove Shared Edges");
            // divideObject not in api - needs zenocore geometryutil
            auto output = zeno::divideObject(...);
            ptrNodeData->set_output_object("Output", output);
            return ZErr_OK;
#else
            ptrNodeData->report_error("Divide: requires divideObject API from zenocore geometryutil");
            return ZErr_UnimplError;
#endif
        }
    };

    ZENDEFNODE_ABI(Divide,
        Z_INPUTS(
            { "Input", _gParamType_Geometry },
            { "Size", _gParamType_Vec3f, ZVec3f(0,0,0) },
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

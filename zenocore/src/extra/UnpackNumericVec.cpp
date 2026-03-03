//
// UnpackNumericVec - INode2 implementation
//
#include <zeno/core/ZNodeParams.h>
#include <zeno/core/defNode.h>
#include <zeno/core/typeinfo.h>
#include <zeno/utils/vec.h>
#include <inodedata.h>
#include <inodeimpl.h>

namespace zeno {

struct UnpackNumericVec : INode2 {
    DEF_OVERRIDE_FOR_INODE
    ZErrorCode apply(INodeData* nd) override {
        auto* params = static_cast<ZNodeParams*>(nd);
        zeno::reflect::Any val = params->get_param_result("vec");
        float x = 0, y = 0, z = 0, w = 0;

        ParamType valType = val.type().hash_code();
        if (gParamType_Float == valType) {
            x = zeno::reflect::any_cast<float>(val);
        } else if (gParamType_Vec2f == valType) {
            auto v = zeno::reflect::any_cast<zeno::vec2f>(val);
            x = v[0]; y = v[1];
        } else if (gParamType_Vec3f == valType) {
            auto v = zeno::reflect::any_cast<zeno::vec3f>(val);
            x = v[0]; y = v[1]; z = v[2];
        } else if (gParamType_Vec4f == valType) {
            auto v = zeno::reflect::any_cast<zeno::vec4f>(val);
            x = v[0]; y = v[1]; z = v[2]; w = v[3];
        } else if (gParamType_Vec2i == valType) {
            auto v = zeno::reflect::any_cast<zeno::vec2i>(val);
            x = (float)v[0]; y = (float)v[1];
        } else if (gParamType_Vec3i == valType) {
            auto v = zeno::reflect::any_cast<zeno::vec3i>(val);
            x = (float)v[0]; y = (float)v[1]; z = (float)v[2];
        } else if (gParamType_Vec4i == valType) {
            auto v = zeno::reflect::any_cast<zeno::vec4i>(val);
            x = (float)v[0]; y = (float)v[1]; z = (float)v[2]; w = (float)v[3];
        } else if (gParamType_Int == valType) {
            x = (float)zeno::reflect::any_cast<int>(val);
        } else {
            nd->report_error("UnpackNumericVec: unsupported input type");
            return ZErr_ParamError;
        }

        nd->set_output_float("X", x);
        nd->set_output_float("Y", y);
        nd->set_output_float("Z", z);
        nd->set_output_float("W", w);
        return ZErr_OK;
    }
};

ZENDEFNODE(UnpackNumericVec, {
    {{gParamType_AnyNumeric, "vec"}},
    {{gParamType_Float, "X"}, {gParamType_Float, "Y"},
     {gParamType_Float, "Z"}, {gParamType_Float, "W"}},
    {},
    {"numeric"},
});

} // namespace zeno

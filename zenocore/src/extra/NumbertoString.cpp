//
// NumbertoString - INode2 implementation
//
#include <zeno/core/ZNodeParams.h>
#include <zeno/core/defNode.h>
#include <zeno/core/typeinfo.h>
#include <zeno/utils/to_string.h>
#include <inodedata.h>
#include <inodeimpl.h>

namespace zeno {

struct NumbertoString : INode2 {
    DEF_OVERRIDE_FOR_INODE
    ZErrorCode apply(INodeData* nd) override {
        auto* params = static_cast<ZNodeParams*>(nd);
        zeno::reflect::Any val = params->get_param_result("number");
        std::string str;

        ParamType valType = val.type().hash_code();
        if (gParamType_Int == valType) {
            str = zeno::to_string(zeno::reflect::any_cast<int>(val));
        } else if (gParamType_Float == valType) {
            str = zeno::to_string(zeno::reflect::any_cast<float>(val));
        } else if (gParamType_Vec2i == valType) {
            str = zeno::to_string(zeno::reflect::any_cast<zeno::vec2i>(val));
        } else if (gParamType_Vec2f == valType) {
            str = zeno::to_string(zeno::reflect::any_cast<zeno::vec2f>(val));
        } else if (gParamType_Vec3i == valType) {
            str = zeno::to_string(zeno::reflect::any_cast<zeno::vec3i>(val));
        } else if (gParamType_Vec3f == valType) {
            str = zeno::to_string(zeno::reflect::any_cast<zeno::vec3f>(val));
        } else if (gParamType_Vec4i == valType) {
            str = zeno::to_string(zeno::reflect::any_cast<zeno::vec4i>(val));
        } else if (gParamType_Vec4f == valType) {
            str = zeno::to_string(zeno::reflect::any_cast<zeno::vec4f>(val));
        } else {
            nd->report_error("NumbertoString: unsupported numeric type");
            return ZErr_ParamError;
        }
        nd->set_output_string("string", str.c_str());
        return ZErr_OK;
    }
};

ZENDEFNODE(NumbertoString, {
    {{gParamType_AnyNumeric, "number"}},
    {{gParamType_String, "string"}},
    {},
    {"string"},
});

} // namespace zeno

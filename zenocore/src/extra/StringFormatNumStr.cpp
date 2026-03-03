//
// StringFormatNumStr - INode2 implementation
//
#include <zeno/core/ZNodeParams.h>
#include <zeno/core/defNode.h>
#include <zeno/core/typeinfo.h>
#include <zeno/utils/format.h>
#include <inodedata.h>
#include <inodeimpl.h>

namespace zeno {

struct StringFormatNumStr : INode2 {
    DEF_OVERRIDE_FOR_INODE
    ZErrorCode apply(INodeData* nd) override {
        auto* params = static_cast<ZNodeParams*>(nd);
        char buf[4096];
        nd->get_input2_string("str", buf, sizeof(buf));
        std::string str(buf);
        zeno::reflect::Any val = params->get_param_result("num_str");
        std::string output;

        ParamType valType = val.type().hash_code();
        if (gParamType_Int == valType) {
            auto v = zeno::reflect::any_cast<int>(val);
            output = zeno::format(str, v);
        } else if (gParamType_Float == valType) {
            auto v = zeno::reflect::any_cast<float>(val);
            output = zeno::format(str, v);
        } else if (gParamType_String == valType) {
            auto v = zeno::reflect::any_cast<std::string>(val);
            output = zeno::format(str, v);
        } else if (gParamType_Vec2i == valType) {
            auto v = zeno::reflect::any_cast<zeno::vec2i>(val);
            output = zeno::format(str, v);
        } else if (gParamType_Vec2f == valType) {
            auto v = zeno::reflect::any_cast<zeno::vec2f>(val);
            output = zeno::format(str, v);
        } else if (gParamType_Vec3i == valType) {
            auto v = zeno::reflect::any_cast<zeno::vec3i>(val);
            output = zeno::format(str, v);
        } else if (gParamType_Vec3f == valType) {
            auto v = zeno::reflect::any_cast<zeno::vec3f>(val);
            output = zeno::format(str, v);
        } else if (gParamType_Vec4i == valType) {
            auto v = zeno::reflect::any_cast<zeno::vec4i>(val);
            output = zeno::format(str, v);
        } else if (gParamType_Vec4f == valType) {
            auto v = zeno::reflect::any_cast<zeno::vec4f>(val);
            output = zeno::format(str, v);
        } else {
            nd->report_error("StringFormatNumStr: unsupported type");
            return ZErr_ParamError;
        }
        nd->set_output_string("str", output.c_str());
        return ZErr_OK;
    }
};

ZENDEFNODE(StringFormatNumStr, {
    {{gParamType_String, "str", "{}"},
     {gParamType_AnyNumeric, "num_str"}},
    {{gParamType_String, "str"}},
    {},
    {"string"},
});

} // namespace zeno

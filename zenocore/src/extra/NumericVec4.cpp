//
// NumericVec4 - INode2 implementation
//
#include <zeno/core/defNode.h>
#include <zeno/core/typeinfo.h>
#include <zeno/utils/interfaceutil.h>
#include <zeno/utils/vec.h>
#include <inodedata.h>
#include <inodeimpl.h>

namespace zeno {

struct NumericVec4 : INode2 {
    DEF_OVERRIDE_FOR_INODE
    ZErrorCode apply(INodeData* nd) override {
        float x = nd->get_input2_float("x");
        float y = nd->get_input2_float("y");
        float z = nd->get_input2_float("z");
        float w = nd->get_input2_float("w");
        nd->set_output_vec4f("vec4", zeno::toAbiVec4f(zeno::vec4f{x, y, z, w}));
        return ZErr_OK;
    }
};

ZENDEFNODE(NumericVec4, {
    {},
    {{gParamType_Vec4f, "vec4"}},
    {{gParamType_Float, "x", "0"}, {gParamType_Float, "y", "0"},
     {gParamType_Float, "z", "0"}, {gParamType_Float, "w", "0"}},
    {"numeric"},
});

} // namespace zeno

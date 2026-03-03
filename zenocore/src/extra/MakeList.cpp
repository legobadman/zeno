//
// MakeList - INode2 implementation
//
#include <zeno/core/defNode.h>
#include <zeno/core/typeinfo.h>
#include <inodedata.h>
#include <inodeimpl.h>

namespace zeno {

struct MakeList : INode2 {
    DEF_OVERRIDE_FOR_INODE
    ZErrorCode apply(INodeData* nd) override {
        IObject2* raw = nd->clone_input_object("objs");
        nd->set_output_object("list", raw);
        return ZErr_OK;
    }
};

ZENDEFNODE(MakeList, {
    {{gParamType_List, "objs", "", zeno::Socket_ReadOnly}},
    {{gParamType_List, "list"}},
    {},
    {"list"},
});

} // namespace zeno

//
// GetFrameNum - INode2 implementation
//
#include <zeno/core/defNode.h>
#include <zeno/core/typeinfo.h>
#include <inodedata.h>
#include <inodeimpl.h>

namespace zeno {

struct GetFrameNum : INode2 {
    DEF_OVERRIDE_FOR_INODE
    ZErrorCode apply(INodeData* nd) override {
        nd->set_output_int("FrameNum", nd->GetFrameId());
        return ZErr_OK;
    }
};

ZENDEFNODE(GetFrameNum, {
    {},
    {{gParamType_Int, "FrameNum"}},
    {},
    {"frame"},
});

} // namespace zeno

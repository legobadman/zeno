//
// StringListGetItem - INode2 implementation
// Gets element at index from string arr, outputs as string
//
#include <zeno/core/defNode.h>
#include <zeno/core/typeinfo.h>
#include <inodedata.h>
#include <inodeimpl.h>
#include <cstring>

namespace zeno {

struct StringListGetItem : INode2 {
    DEF_OVERRIDE_FOR_INODE
    ZErrorCode apply(INodeData* nd) override {
        size_t count = nd->get_input_string_list_count("list");
        if (count == 0) {
            nd->report_error("StringListGetItem: input list is empty or null");
            return ZErr_ParamError;
        }

        int index = nd->get_input2_int("index");
        if (index < 0) {
            index += (int)count;
        }
        if (index < 0 || (size_t)index >= count) {
            nd->report_error("StringListGetItem: index out of range");
            return ZErr_ParamError;
        }

        char buf[4096];
        nd->get_input_string_list("list", (size_t)index, buf, sizeof(buf));
        nd->set_output_string("string", buf);
        return ZErr_OK;
    }
};

ZENDEFNODE(StringListGetItem, {
    {
        {gParamType_StringList, "list"},
        {gParamType_Int, "index", "0"},
    },
    {{gParamType_String, "string"}},
    {},
    {"string"},
});

} // namespace zeno

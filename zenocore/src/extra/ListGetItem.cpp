//
// ListGetItem - INode2 implementation
//
#include <zeno/core/defNode.h>
#include <zeno/core/typeinfo.h>
#include <inodedata.h>
#include <inodeimpl.h>

namespace zeno {

struct ListGetItem : INode2 {
    DEF_OVERRIDE_FOR_INODE
    ZErrorCode apply(INodeData* nd) override {
        int index = nd->get_input2_int("index");
        IObject2* listRaw = nd->get_input_object("list");
        if (!listRaw) {
            nd->report_error("ListGetItem: no input list");
            return ZErr_ParamError;
        }
        IListObject* list = dynamic_cast<IListObject*>(listRaw);
        if (list) {
            if (index < 0)
                index += (int)list->size();
            if (index < 0 || index >= (int)list->size()) {
                nd->report_error("ListGetItem: index out of range");
                return ZErr_ParamError;
            }
            IObject2* obj = list->get(index)->clone();
            nd->set_output_object("object", obj);
        } else {
            nd->report_error("ListGetItem: input is not a list (Dict not yet supported in zenocore)");
            return ZErr_ParamError;
        }
        return ZErr_OK;
    }
};

ZENDEFNODE(ListGetItem, {
    {{gParamType_List, "list", "", zeno::Socket_ReadOnly, NoMultiSockPanel},
     {gParamType_Int, "index"}},
    {{_gParamType_IObject, "object"}},
    {},
    {"list"},
});

} // namespace zeno

#include <zeno/zeno.h>

namespace zeno
{
    struct Duplicate : INode
    {
        void apply() override {
            zany input_object = ZImpl(get_input("Original Object"));
            bool keepModifyOriginal = ZImpl(get_input2<bool>("Keep Modify Original"));

            zany first_clone = input_object->clone();
            if (keepModifyOriginal) {
                ZImpl(set_output("Original Object", input_object));
            }
            ZImpl(set_output("Duplicated Object", first_clone));
        }
    };

    ZENDEFNODE(Duplicate,
    {
        {
            {gParamType_IObject, "Original Object"},
            {gParamType_Bool, "Keep Modify Original"}
        },
        {
            {gParamType_IObject, "Duplicated Object"},
            ParamObject("Original Object", gParamType_IObject, "visible = param('Keep Modify Original').value == 1;")
        },
        {},
        {"geom"}
    });
}
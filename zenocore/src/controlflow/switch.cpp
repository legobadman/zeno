#include <zeno/zeno.h>
#include <zeno/types/ListObject_impl.h>
#include <zeno/utils/interfaceutil.h>


namespace zeno
{
    struct Switch : INode
    {
        void apply() override {
            auto input_objects = ZImpl(get_input2<ListObject>("Input Objects"));
            if (!input_objects) {
                throw makeError<UnimplError>("no input objects on Switch");
            }
            int switch_num = ZImpl(get_input2<int>("Switch Number"));
            int n = input_objects->m_impl->size();
            int clip_switch = std::min(n - 1, std::max(0, switch_num));
            zany obj = input_objects->m_impl->get(clip_switch);
            ZImpl(set_output("Output Object", obj));
        }
    };

    ZENDEFNODE(Switch,
    {
        {
            {gParamType_List, "Input Objects"},
            {gParamType_Int, "Switch Number"}
        },
        {
            {gParamType_Geometry, "Output Object"}
        },
        {},
        {"reflect"}
    });

    //TODO:
    //struct SwitchIf : INode
    //{
    //};
}

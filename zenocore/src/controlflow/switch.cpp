#include <zeno/zeno.h>
#include <zeno/types/ListObject_impl.h>
#include <zeno/utils/interfaceutil.h>
#include <zeno/core/ZNodeParams.h>


namespace zeno
{
    struct Switch : INode2
    {
        ZErrorCode apply(INodeData* pNodeData) override {
            auto input_objects = pNodeData->get_input_ListObject("Input Objects");
            if (!input_objects) {
                throw makeError<UnimplError>("no input objects on Switch");
            }
            int switch_num = pNodeData->get_input2_int("Switch Number");
            int n = input_objects->size();
            int clip_switch = std::min(n - 1, std::max(0, switch_num));
            auto obj = input_objects->get(clip_switch);
            pNodeData->set_output_object("Output Object", obj->clone());
            return ZErr_OK;
        }

        DEF_OVERRIDE_FOR_INODE
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

    struct NumericIf : INode2
    {
        ZErrorCode apply(INodeData* pNodeData) override {
            auto m_pAdapter = static_cast<ZNodeParams*>(pNodeData);
            int cond = m_pAdapter->get_input2_int("Condition");
            if (cond != 0) {
                m_pAdapter->set_primitive_output("Output", m_pAdapter->get_param_result("If True"));
            }
            else {
                m_pAdapter->set_primitive_output("Output", m_pAdapter->get_param_result("If False"));
            }
            return ZErr_OK;
        }

        NodeType type() const { return Node_Normal; }
        void clearCalcResults() {}
        void getIconResource(char* recv, size_t cap) {
            const char* icon = ":/icons/node/switchif.svg";
            strcpy(recv, icon);
            recv[strlen(icon)] = '\0';
        }
        void getBackgroundClr(char* recv, size_t cap) {
            const char* bg = "#CEFFB3";
            strcpy(recv, bg);
            recv[strlen(bg)] = '\0';
        }
        float time() const { return 1.0f; }
    };

    ZENDEFNODE(NumericIf,
    {
        {
            {gParamType_AnyNumeric, "If False"},
            {gParamType_AnyNumeric, "If True"},
            {gParamType_Int, "Condition"}
        },
        {
            {gParamType_AnyNumeric, "Output"}
        },
        {},
        {"flow"}
    });


    struct SwitchIf : INode2
    {
        ZErrorCode apply(INodeData* _pNodeData) override {
            auto pNodeData = static_cast<ZNodeParams*>(_pNodeData);
            int cond = pNodeData->get_input2_int("Condition");
            if (cond != 0) {
                pNodeData->set_output("Output", pNodeData->clone_input("If True"));
            }
            else {
                pNodeData->set_output("Output", pNodeData->clone_input("If False"));
            }
            return ZErr_OK;
        }

        NodeType type() const { return Node_Normal; }
        void clearCalcResults() {}
        void getIconResource(char* recv, size_t cap) {
            const char* icon = ":/icons/node/switchif.svg";
            strcpy(recv, icon);
            recv[strlen(icon)] = '\0';
        }
        void getBackgroundClr(char* recv, size_t cap) {
            const char* bg = "#CEFFB3";
            strcpy(recv, bg);
            recv[strlen(bg)] = '\0';
        }
        float time() const { return 1.0f; }
    };

    ZENDEFNODE(SwitchIf,
    {
        {
            {gParamType_IObject2, "If False"},
            {gParamType_IObject2, "If True"},
            {gParamType_Int, "Condition"}
        },
        {
            {gParamType_IObject2, "Output"}
        },
        {},
        {"flow"}
    });

    struct SwitchBetween : INode2
    {
        ZErrorCode apply(INodeData* _pNodeData) override {
            auto pNodeData = static_cast<ZNodeParams*>(_pNodeData);
            int cond = pNodeData->get_input2_int("cond1");
            if (cond != 0) {
                pNodeData->set_output("Output", pNodeData->clone_input("b1"));
                return ZErr_OK;
            }

            cond = pNodeData->get_input2_int("cond2");
            if (cond != 0) {
                pNodeData->set_output("Output", pNodeData->clone_input("b2"));
                return ZErr_OK;
            }

            cond = pNodeData->get_input2_int("cond3");
            if (cond != 0) {
                pNodeData->set_output("Output", pNodeData->clone_input("b3"));
                return ZErr_OK;
            }

            cond = pNodeData->get_input2_int("cond4");
            if (cond != 0) {
                pNodeData->set_output("Output", pNodeData->clone_input("b4"));
                return ZErr_OK;
            }

            cond = pNodeData->get_input2_int("cond5");
            if (cond != 0) {
                pNodeData->set_output("Output", pNodeData->clone_input("b5"));
                return ZErr_OK;
            }

            cond = pNodeData->get_input2_int("cond6");
            if (cond != 0) {
                pNodeData->set_output("Output", pNodeData->clone_input("b6"));
                return ZErr_OK;
            }

            cond = pNodeData->get_input2_int("cond7");
            if (cond != 0) {
                pNodeData->set_output("Output", pNodeData->clone_input("b7"));
                return ZErr_OK;
            }

            cond = pNodeData->get_input2_int("cond8");
            if (cond != 0) {
                pNodeData->set_output("Output", pNodeData->clone_input("b8"));
                return ZErr_OK;
            }
            return ZErr_OK;
        }

        NodeType type() const {
            return Node_SubgraphNode;
        }
        void clearCalcResults() {}
        void getIconResource(char* recv, size_t cap) {
            const char* icon = ":/icons/node/switch-between.svg";
            strcpy(recv, icon);
            recv[strlen(icon)] = '\0';
        }
        void getBackgroundClr(char* recv, size_t cap) {
            const char* bg = "#CEFFB3";
            strcpy(recv, bg);
            recv[strlen(bg)] = '\0';
        }
        float time() const { return 1.0f; }
    };

    ZENDEFNODE(SwitchBetween,
    {
        {
            {gParamType_IObject2, "b1"},
            {gParamType_IObject2, "b2"},
            {gParamType_IObject2, "b3"},
            {gParamType_IObject2, "b4"},
            {gParamType_IObject2, "b5"},
            {gParamType_IObject2, "b6"},
            {gParamType_IObject2, "b7"},
            {gParamType_IObject2, "b8"},
            {gParamType_Int, "cond1", "0"},
            {gParamType_Int, "cond2", "0"},
            {gParamType_Int, "cond3", "0"},
            {gParamType_Int, "cond4", "0"},
            {gParamType_Int, "cond5", "0"},
            {gParamType_Int, "cond6", "0"},
            {gParamType_Int, "cond7", "0"},
            {gParamType_Int, "cond8", "0"},
        },
        {
            {gParamType_IObject2, "Output"}
        },
        {},
        {"flow"}
    });
}

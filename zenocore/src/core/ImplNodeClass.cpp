#include <zeno/zeno.h>
#include <zeno/core/INodeClass.h>
#include <regex>
#include <zeno/utils/helper.h>

namespace zeno {

    static void initCoreParams(INodeImpl* spNode, CustomUI customui)
    {
        //init all params, and set defl value
        for (const ParamObject& param : customui.inputObjs)
        {
            spNode->m_pImpl->add_input_obj_param(param);
        }
        for (const ParamPrimitive& param : customUiToParams(customui.inputPrims))
        {
            spNode->m_pImpl->add_input_prim_param(param);
        }
        for (const ParamPrimitive& param : customui.outputPrims)
        {
            spNode->m_pImpl->add_output_prim_param(param);
        }
        for (const ParamObject& param : customui.outputObjs)
        {
            spNode->m_pImpl->add_output_obj_param(param);
        }
        //根据customui上的约束信息调整所有控件的可见可用情况
        spNode->m_pImpl->checkParamsConstrain();
    }


    ImplNodeClass::ImplNodeClass(INode*(*ctor)(), CustomUI const& customui, std::string const& name)
        : INodeClass(customui, name), ctor(ctor) {}

    std::unique_ptr<INodeImpl> ImplNodeClass::new_instance(Graph* pGraph, std::string const& name) {
        std::unique_ptr<INodeImpl> spNode = std::make_unique<INodeImpl>(ctor());
        spNode->m_pImpl->initUuid(pGraph, classname);
        spNode->m_pImpl->set_name(name);
        initCoreParams(spNode.get(), m_customui);
        return spNode;
    }

}

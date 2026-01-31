#include <zeno/zeno.h>
#include <zeno/core/INodeClass.h>
#include <regex>
#include <zeno/utils/helper.h>
#include <zeno/extra/SubnetNode.h>
#include <zeno/core/SolverImpl.h>

namespace zeno {

    static void initCoreParams(NodeImpl* spNode, CustomUI customui)
    {
        //init all params, and set defl value
        for (const ParamObject& param : customui.inputObjs)
        {
            spNode->add_input_obj_param(param);
        }
        for (const ParamPrimitive& param : customUiToParams(customui.inputPrims))
        {
            spNode->add_input_prim_param(param);
        }
        for (const ParamPrimitive& param : customui.outputPrims)
        {
            spNode->add_output_prim_param(param);
        }
        for (const ParamObject& param : customui.outputObjs)
        {
            spNode->add_output_obj_param(param);
        }
        //根据customui上的约束信息调整所有控件的可见可用情况
        spNode->checkParamsConstrain();
    }


    ImplNodeClass::ImplNodeClass(INode2*(*ctor)(), void (*dtor)(INode2*), CustomUI const& customui, std::string const& name)
        : INodeClass(customui, name), ctor(ctor), dtor(dtor) {}

    std::unique_ptr<NodeImpl> ImplNodeClass::new_instance(Graph* pGraph, std::string const& name) {
        INode2* pNode = ctor();
        std::unique_ptr<NodeImpl> spNode;
        NodeType type = pNode->type();
        if (type == Node_SubgraphNode) {
            auto subnetNode = std::make_unique<SubnetNode>(pNode, dtor);
            spNode = std::move(subnetNode);
        }
        else if (type == Node_Solver) {
            auto solverNode = std::make_unique<SolverImpl>(pNode, dtor);
            spNode = std::move(solverNode);
        }
        else {
            spNode = std::make_unique<NodeImpl>(pNode, dtor);
        }
        spNode->initUuid(pGraph, classname);
        spNode->set_name(name);
        initCoreParams(spNode.get(), m_customui);
        return spNode;
    }

    std::unique_ptr<INode2> ImplNodeClass::new_coreinst() {
        std::unique_ptr<INode2> upNode(ctor());
        return upNode;
    }
}

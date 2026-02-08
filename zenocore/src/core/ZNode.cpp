#include <zeno/core/ZNode.h>
#include <zeno/core/ZNodeExecutor.h>
#include <zeno/core/ZNodeParams.h>
#include <zeno/core/ZNodeStatus.h>


namespace zeno {

    ZNode::ZNode(
        const std::string& class_name,
        const std::string& name,
        INode2* pNode,
        void (*dtor)(INode2*),
        Graph* pGraph,
        const CustomUI& customui
    )
    {
        m_upParams = std::make_unique<ZNodeParams>(customui);
        m_upNodeExec = std::make_unique<ZNodeExecutor>(this, pNode, dtor);
        m_upNodeStatus = std::make_unique<ZNodeStatus>(this, class_name, name);
    }

    ZNodeParams* ZNode::getNodeParams() const {
        return m_upParams.get();
    }

    ZNodeExecutor* ZNode::getNodeExecutor() const {
        return m_upNodeExec.get();
    }

    ZNodeStatus* ZNode::getNodeStatus() const {
        return m_upNodeStatus.get();
    }
}
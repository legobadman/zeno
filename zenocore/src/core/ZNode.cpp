#include <zeno/core/ZNode.h>


namespace zeno {

    ZNode::ZNode(
        const std::string& class_name,
        const std::string& name,
        INode2* pNode,
        void (*dtor)(INode2*),
        Graph* pGraph,
        const CustomUI& customui
    )
        : m_upParams(this, customui)
        , m_upNodeExec(this, pNode, dtor)
        , m_upNodeStatus(this, class_name, name)
    {
    }

    void ZNode::init(const NodeData& dat) {
        m_upNodeStatus.initDat(dat);
        m_upParams.initParams(dat);
        m_upNodeExec.initAfterIO();
    }

    ZNodeParams& ZNode::getNodeParams() {
        return m_upParams;
    }

    ZNodeExecutor& ZNode::getNodeExecutor() {
        return m_upNodeExec;
    }

    ZNodeStatus& ZNode::getNodeStatus() {
        return m_upNodeStatus;
    }

    std::string ZNode::get_name() const {
        return m_upNodeStatus.get_name();
    }

    std::string ZNode::get_path() const {
        return m_upNodeStatus.get_path();
    }

    std::string ZNode::get_uuid() const {
        return m_upNodeStatus.get_uuid();
    }
}
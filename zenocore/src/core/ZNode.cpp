#include <zeno/core/ZNode.h>
#include <zeno/core/Session.h>
#include <zeno/core/Assets.h>


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
        if (is_subnet()) {
            m_opt_subnet_info->init_data(dat);
        }
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

    std::optional<ZSubnetInfo>& ZNode::getSubnetInfo() {
        return m_opt_subnet_info;
    }

    void ZNode::initSubnetInfo() {
        //这里的初始化针对的是创建，如果是读写，可能另有流程
        m_opt_subnet_info = ZSubnetInfo(this);
        auto subnetui = m_opt_subnet_info->init_subnet_ui();
        m_upParams.setCustomUi(subnetui, true);
    }

    bool ZNode::is_subnet() const {
        return m_opt_subnet_info.has_value();
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

    std::string ZNode::get_nodecls() const {
        return m_upNodeStatus.get_nodecls();
    }

    bool ZNode::in_asset_file() const {
        Graph* spGraph = m_upNodeStatus.getGraph();
        assert(spGraph);
        return getSession().assets->isAssetGraph(spGraph);
    }

    NodeData ZNode::exportInfo() const {
        NodeData node;
        node.cls = m_upNodeStatus.get_nodecls();
        node.name = m_upNodeStatus.get_name();
        node.bView = m_upNodeStatus.is_view();
        node.uipos = m_upNodeStatus.get_pos();
        node.bnocache = m_upNodeStatus.is_nocache();
        //TODO: node type
        if (node.subgraph.has_value())
            node.type = Node_SubgraphNode;
        else
            node.type = Node_Normal;
        node.bLocked = false;
        node.customUi = m_upParams.exportCuiWithValue();

        if (is_subnet()) {
            node.bclearsbn = m_opt_subnet_info->is_clearsubnet();
            const Asset& asset = zeno::getSession().assets->getAsset(node.cls);
            if (!asset.m_info.name.empty()) {
                node.asset = asset.m_info;
                if (in_asset_file()) {
                    node.type = Node_AssetReference;
                    node.bLocked = true;    //资产图里的资产只是引用，故不能展开，自然不能解锁
                }
                else {
                    node.type = Node_AssetInstance;
                    node.bLocked = m_opt_subnet_info->is_locked();
                    if (!node.bLocked) {
                        node.subgraph = m_opt_subnet_info->get_subgraph()->exportGraph();
                    }
                }
            }
            else {
                node.subgraph = m_opt_subnet_info->get_subgraph()->exportGraph();
                node.type = Node_SubgraphNode;
            }
        }
        return node;
    }

    void ZNode::on_node_about_to_remove() {
        //如果有回调注册了，就先处理回调，让注册方完成释放操作
        for (const auto& [_, cb] : m_cbRemoveSelfCallback) {
            cb();
        }
        m_upParams.removeRefLinks();
    }
}
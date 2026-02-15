#include <zeno/core/ZSubnetInfo.h>
#include <zeno/core/ZNode.h>
#include <zeno/core/Assets.h>


namespace zeno {

    ZSubnetInfo::ZSubnetInfo(ZNode* pNode) : m_pNode(pNode) {

    }

    Graph* ZSubnetInfo::get_subgraph() const {
        return m_subgraph.get();
    }

    void ZSubnetInfo::init_graph(std::unique_ptr<Graph>&& subg) {
        m_subgraph = std::move(subg);
    }

    bool ZSubnetInfo::isAssetsNode() const {
        zeno::Asset asst = zeno::getSession().assets->getAsset(m_pNode->getNodeStatus().get_nodecls());
        return !asst.m_info.name.empty();
    }

    NodeType ZSubnetInfo::nodeType() const {
        if (isAssetsNode()) {
            if (m_pNode->in_asset_file())
                return zeno::Node_AssetReference;
            else
                return zeno::Node_AssetInstance;
        }
        return zeno::Node_SubgraphNode;
    }

    bool ZSubnetInfo::is_locked() const {
        if (nodeType() == Node_AssetInstance || nodeType() == Node_AssetReference)
            return m_bLocked;
        else
            return false;
    }

    void ZSubnetInfo::set_locked(bool bLocked) {
        m_bLocked = bLocked;
    }

    bool ZSubnetInfo::is_clearsubnet() const {
        return m_bClearSubnet;
    }

    void ZSubnetInfo::set_clearsubnet(bool bOn) {
        m_bClearSubnet = bOn;
    }

}
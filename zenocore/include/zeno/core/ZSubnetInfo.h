#pragma once

#include <zeno/core/Session.h>
#include <zeno/core/Graph.h>
#include <zeno/core/Descriptor.h>
#include <zeno/utils/uuid.h>
#include <memory>

namespace zeno
{
    class ZNode;

    class ZENO_API ZSubnetInfo {
    public:
        ZSubnetInfo(ZNode*);
        Graph* get_subgraph() const;
        void init_graph(std::unique_ptr<Graph>&& subg);
        void init_data(const NodeData& dat);
        CustomUI init_subnet_ui() const;
        bool isAssetsNode() const;
        NodeType nodeType() const;
        void mark_subnetdirty(bool bOn);

        bool is_locked() const;
        void set_locked(bool bLocked);
        CALLBACK_REGIST(lockChanged, void, void)

        bool is_clearsubnet() const;
        void set_clearsubnet(bool bOn);
        CALLBACK_REGIST(clearSubnetChanged, void, bool)

    private:
        ZNode* m_pNode{ nullptr };
        std::unique_ptr<Graph> m_subgraph;    //assets subnetnode
        bool m_bLocked{ false };
        bool m_bClearSubnet{ false };
    };
}
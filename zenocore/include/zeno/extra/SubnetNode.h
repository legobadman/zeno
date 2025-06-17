#pragma once

#include <zeno/core/NodeImpl.h>
#include <zeno/core/Session.h>
#include <zeno/core/Graph.h>
#include <zeno/core/Descriptor.h>
#include <zeno/utils/uuid.h>

namespace zeno {

struct ZENO_API SubnetNode : NodeImpl {

    SubnetNode(INode* pNode);
    ~SubnetNode();

    void apply() override;

    void initParams(const NodeData& dat) override;
    params_change_info update_editparams(const ParamsUpdateInfo& params, bool bSubnetInit = false) override;
    Graph* get_subgraph() const;
    void init_graph(std::shared_ptr<Graph> subg);
    bool isAssetsNode() const;
    bool is_loaded() const override;
    bool is_locked() const override;
    void set_locked(bool bLocked) override;
    NodeType nodeType() const override;

    NodeData exportInfo() const override;
    CustomUI get_customui() const override;
    CustomUI export_customui() const override;

    void setCustomUi(const CustomUI& ui);
    void mark_subnetdirty(bool bOn);

    CALLBACK_REGIST(lockChanged, void, void)

protected:
    CustomUI m_customUi;
    std::shared_ptr<Graph> m_subgraph;    //assets subnetnode

private:
    bool m_bLocked; //只给资产用
};

struct ZENO_API DopNetwork : zeno::SubnetNode {

    DopNetwork();
    void apply() override;

    void setEnableCache(bool enable);
    void setAllowCacheToDisk(bool enable);
    void setMaxCacheMemoryMB(int size);
    void setCurrCacheMemoryMB(int size);
    static size_t getObjSize(IObject* obj);
    void resetFrameState();

    CALLBACK_REGIST(dopnetworkFrameRemoved, void, int)
    CALLBACK_REGIST(dopnetworkFrameCached, void, int)

    bool m_bEnableCache;
    bool m_bAllowCacheToDisk;
    int m_maxCacheMemoryMB;
    int m_currCacheMemoryMB;

    std::map<int, std::map<std::string, std::shared_ptr<zeno::IObject>>> m_frameCaches;
    std::map<int, size_t> m_frameCacheSizes;
    size_t m_totalCacheSizeByte;
};

}

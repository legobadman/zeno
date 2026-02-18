#include <zeno/core/ZSubnetInfo.h>
#include <zeno/core/ZNode.h>
#include <zeno/core/Assets.h>
#include <zeno/core/typeinfo.h>
#include "zeno_types/reflect/reflection.generated.hpp"


namespace zeno {

    ZSubnetInfo::ZSubnetInfo(ZNode* pNode)
        : m_pNode(pNode)
        , m_subgraph(std::make_unique<Graph>(""))
    {
        m_subgraph->initParentSubnetNode(m_pNode);
    }

    Graph* ZSubnetInfo::get_subgraph() const {
        return m_subgraph.get();
    }

    void ZSubnetInfo::init_graph(std::unique_ptr<Graph>&& subg) {
        m_subgraph = std::move(subg);
    }

    void ZSubnetInfo::init_data(const NodeData& dat) {
        m_bClearSubnet = dat.bclearsbn;
        if (dat.subgraph)
            m_subgraph->init(*dat.subgraph);
        if (zeno::Node_AssetInstance == nodeType()) {
            m_bLocked = dat.bLocked;
        }
    }

    CustomUI ZSubnetInfo::init_subnet_ui() const {
        //init some params
        CustomUI subnetui;  //也可以从subnet节点定义获得初始化？
        zeno::ParamTab tab;
        zeno::ParamGroup default_group;

        zeno::ParamUpdateInfo info;

        zeno::ParamPrimitive param;
        param.bInput = true;
        param.name = "data_input";
        param.defl = zeno::reflect::make_any<zeno::PrimVar>(zeno::PrimVar(0));;
        param.type = zeno::types::gParamType_Int;
        param.socketType = zeno::Socket_Primitve;
        param.control = zeno::Lineedit;
        param.bSocketVisible = false;
        info.param = param;
        default_group.params.push_back(param);

        zeno::ParamPrimitive outputparam;
        outputparam.bInput = false;
        outputparam.name = "data_output";
        outputparam.defl = 2;
        outputparam.type = zeno::types::gParamType_Int;
        outputparam.socketType = zeno::Socket_Primitve;
        outputparam.bSocketVisible = false;
        info.param = outputparam;

        zeno::ParamObject objInput;
        objInput.bInput = true;
        objInput.name = "Input";
        objInput.type = gParamType_Geometry;

        zeno::ParamObject objOutput;
        objOutput.bInput = false;
        objOutput.name = "Output";
        objOutput.type = gParamType_Geometry;
        objOutput.socketType = zeno::Socket_Output;

        tab.groups.emplace_back(std::move(default_group));
        subnetui.inputPrims.emplace_back(std::move(tab));
        subnetui.inputObjs.push_back(objInput);
        subnetui.outputPrims.push_back(outputparam);
        subnetui.outputObjs.push_back(objOutput);

        subnetui.uistyle.background = "#1D5F51";
        subnetui.uistyle.iconResPath = ":/icons/node/subnet.svg";
        return subnetui;
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

    void ZSubnetInfo::mark_subnetdirty(bool bOn)
    {
        if (bOn) {
            m_subgraph->markDirtyAndCleanup();
        }
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
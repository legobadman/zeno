#include <zeno/core/ZNodeStatus.h>
#include <zeno/core/ZNode.h>
#include <zeno/core/Graph.h>
#include <zeno/utils/helper.h>


namespace zeno {

    ZNodeStatus::ZNodeStatus(
        ZNode* pNodeRepo,
        const std::string& class_name,
        const std::string& name)
        : m_name(name)
        , m_nodecls(class_name)
        , m_pNodeRepo(pNodeRepo)
    {
    }

    void ZNodeStatus::initDat(const NodeData& dat)
    {
        //IO init
        if (!dat.name.empty())
            m_name = dat.name;

        if (m_name == "Create_body_base") {
            int j;
            j = -0;
        }

        m_pos = dat.uipos;
        m_bView = dat.bView;
        m_bypass = dat.bypass;
        m_nocache = dat.bnocache;

        if (m_bView) {
            Graph* spGraph = m_pGraph;
            assert(spGraph);
            spGraph->viewNodeUpdated(m_name, m_bView);
        }

        auto& optSbn = m_pNodeRepo->getSubnetInfo();
        if (optSbn.has_value()) {
            optSbn;
        }

        //if (SubnetNode* pSubnetNode = dynamic_cast<SubnetNode*>(this))
        //{
        //    zeno::NodeType nodetype = pSubnetNode->nodeType();
        //    if (nodetype != zeno::Node_AssetInstance && nodetype != zeno::Node_AssetReference) {//asset初始化时已设置过customui
        //        pSubnetNode->setCustomUi(dat.customUi);
        //    }
        //}
        //initParams(dat);
        //m_dirty = true;
        //if (m_nodecls == "FrameCache") {
        //    //有可能上一次已经缓存了内容，如果我们允许上一次的缓存留下来的话，就可以标为FrameChanged
        //    //那就意味着，如果想清理缓存，只能让用户手动清理，因为机制不好理解什么时候缓存失效
        //    m_dirtyReason = Dirty_FrameChanged;
        //}
        //else {
        //    m_dirtyReason = Dirty_All;
        //}
    }

    std::string ZNodeStatus::get_nodecls() const
    {
        return m_nodecls;
    }

    std::string ZNodeStatus::get_ident() const
    {
        return m_name;
    }

    std::string ZNodeStatus::get_show_name() const
    {
        const auto& cui = m_pNodeRepo->getNodeParams().get_customui();
        if (!cui.nickname.empty())
            return cui.nickname;
        return m_nodecls;
    }

    std::string ZNodeStatus::get_show_icon() const
    {
        const auto& cui = m_pNodeRepo->getNodeParams().get_customui();
        return cui.uistyle.iconResPath;
    }

    ObjPath ZNodeStatus::get_path() const
    {
        ObjPath path;
        path = m_name;

        Graph* pGraph = m_pGraph;

        while (pGraph) {
            const std::string name = pGraph->getName();
            if (name == "main") {
                path = "/main/" + path;
                break;
            }
            else {
                path = name + "/" + path;
                if (!pGraph->getParentSubnetNode())
                    break;
                auto pSubnetNode = pGraph->getParentSubnetNode();
                assert(pSubnetNode);
                pGraph = pSubnetNode->getNodeStatus().getGraph();
            }
        }
        return path;
    }

    ObjPath ZNodeStatus::get_graph_path() const
    {
        ObjPath path;
        path = "";

        Graph* pGraph = m_pGraph;

        while (pGraph) {
            const std::string name = pGraph->getName();
            if (name == "main") {
                path = "/main/" + path;
                break;
            }
            else {
                if (!pGraph->getParentSubnetNode())
                    break;
                auto pSubnetNode = pGraph->getParentSubnetNode();
                assert(pSubnetNode);
                path = pSubnetNode->getNodeStatus().get_name() + "/" + path;
                pGraph = pSubnetNode->getNodeStatus().getGraph();
            }
        }
        return path;
    }

    ObjPath ZNodeStatus::get_uuid_path() const
    {
        return m_uuidPath;
    }

    std::string ZNodeStatus::get_uuid() const
    {
        return m_uuid;
    }

    void ZNodeStatus::initUuid(Graph* pGraph, const std::string nodecls)
    {
        //TODO: 考虑asset的情况
        m_nodecls = nodecls;
        this->m_pGraph = pGraph;

        m_uuid = generateUUID(nodecls);
        ObjPath path;
        path += m_uuid;
        while (pGraph) {
            const std::string name = pGraph->getName();
            if (name == "main") {
                break;
            }
            else {
                if (!pGraph->getParentSubnetNode())
                    break;
                auto pSubnetNode = pGraph->getParentSubnetNode();
                assert(pSubnetNode);
                path = (pSubnetNode->getNodeStatus().get_uuid()) + "/" + path;
                pGraph = pSubnetNode->getNodeStatus().getGraph();
            }
        }
        m_uuidPath = path;
    }

    NodeType ZNodeStatus::nodeType() const
    {
        INode2* pImplNode = m_pNodeRepo->getNodeExecutor().coreNode();
        if (pImplNode) {
            return pImplNode->type();
        }
        else {
            //可能是asset
            const auto& optSbnInfo = m_pNodeRepo->getSubnetInfo();
            if (optSbnInfo.has_value() && optSbnInfo->isAssetsNode()) {
                if (m_pNodeRepo->in_asset_file()) {
                    return Node_AssetReference;
                }
                else {
                    return Node_AssetInstance;
                }
            }
            return NoVersionNode;
        }
    }

    void ZNodeStatus::convert_to_assetinst(const std::string& asset_name)
    {
        //原代码也是virtual
        m_nodecls = asset_name;
    }

    void ZNodeStatus::set_view(bool bOn)
    {
        //这一类方法要和ui model端位于同一线程
        if (m_bView == bOn) {
            return;
        }
        {
            CORE_API_BATCH

            m_bView = bOn;
            CALLBACK_NOTIFY(set_view, m_bView)

            Graph* spGraph = m_pGraph;
            assert(spGraph);
            spGraph->viewNodeUpdated(m_name, bOn);
        }
    }

    bool ZNodeStatus::is_view() const
    {
        return m_bView;
    }

    void ZNodeStatus::set_bypass(bool bOn)
    {
        CORE_API_BATCH

        m_bypass = bOn;
        CALLBACK_NOTIFY(set_bypass, m_bypass)
        m_pNodeRepo->getNodeExecutor().mark_dirty(true);
    }

    bool ZNodeStatus::is_bypass() const
    {
        return m_bypass;
    }

    void ZNodeStatus::set_nocache(bool bOn)
    {
        CORE_API_BATCH

        m_nocache = bOn;
        CALLBACK_NOTIFY(set_nocache, m_nocache)
        m_pNodeRepo->getNodeExecutor().mark_dirty(true);
    }

    bool ZNodeStatus::is_nocache() const
    {
        return m_nocache;
    }

    void ZNodeStatus::set_name(const std::string& name)
    {
        m_name = name;
    }

    std::string ZNodeStatus::get_name() const
    {
        return m_name;
    }

    void ZNodeStatus::onNodeNameUpdated(const std::string& oldname, const std::string& newname) {
        std::string graphpath = get_graph_path();
        std::string oldpath = graphpath + '/' + oldname;
        std::string newpath = graphpath + '/' + newname;

        //检查所有reflink，将目标参数的引用名称调整一下
        auto& _inputPrims = m_pNodeRepo->getNodeParams().get_input_prim_params2();
        for (const auto& [_, param] : _inputPrims) {
            for (auto reflink : param.reflinks) {
                if (reflink->dest_inparam != &param) {
                    //直接修改dest_inparam->defl.
                    bool bUpdate = false;

                    auto fUpdateParamDefl = [oldpath, newpath, graphpath, &bUpdate](std::string& arg) {
                        auto matchs = zeno::getReferPath(arg);
                        for (const auto& str : matchs)
                        {
                            std::string absolutePath = zeno::absolutePath(graphpath, str);
                            if (absolutePath.find(oldpath) != std::string::npos)
                            {
                                std::regex num_rgx("[0-9]+");
                                //如果是数字，需要将整个refer替换
                                if (std::regex_match(newpath, num_rgx))
                                {
                                    arg = newpath;
                                    bUpdate = true;
                                    break;
                                }
                                else
                                {
                                    std::regex pattern(oldpath);
                                    std::string format = regex_replace(absolutePath, pattern, newpath);
                                    //relative path
                                    if (absolutePath != str)
                                    {
                                        format = zeno::relativePath(graphpath, format);
                                    }
                                    std::regex rgx(str);
                                    arg = regex_replace(arg, rgx, format);
                                }
                                bUpdate = true;
                            }
                        }
                    };

                    zeno::reflect::Any adjustParamVal = reflink->dest_inparam->defl;

                    assert(adjustParamVal.has_value());
                    ParamType type = adjustParamVal.type().hash_code();
                    if (type == zeno::types::gParamType_PrimVariant) {
                        PrimVar var = zeno::reflect::any_cast<PrimVar>(adjustParamVal);
                        std::visit([&](auto& arg) {
                            using T = std::decay_t<decltype(arg)>;
                            if constexpr (std::is_same_v<T, std::string>) {
                                fUpdateParamDefl(arg);
                            }
                            else {
                                assert(false);
                                zeno::log_warn("error param type");
                            }
                            }, var);
                        if (bUpdate) {
                            adjustParamVal = zeno::reflect::move(var);
                        }
                    }
                    else if (type == zeno::types::gParamType_VecEdit) {
                        vecvar var = zeno::reflect::any_cast<vecvar>(adjustParamVal);
                        for (PrimVar& elem : var)
                        {
                            std::visit([&](auto& arg) {
                                using T = std::decay_t<decltype(arg)>;
                                if constexpr (std::is_same_v<T, std::string>) {
                                    fUpdateParamDefl(arg);
                                }
                                }, elem);
                        }
                        if (bUpdate) {
                            adjustParamVal = zeno::reflect::move(var);
                        }
                    }
                    else {
                        assert(false);
                        zeno::log_error("unknown param type of refer param");
                    }

                    if (bUpdate) {
                        auto spDestNode = reflink->dest_inparam->m_wpNode;
                        spDestNode->getNodeParams().update_param(reflink->dest_inparam->name, adjustParamVal);
                    }
                }
            }
        }
    }

    void ZNodeStatus::set_pos(std::pair<float, float> pos)
    {
        m_pos = pos;
        CALLBACK_NOTIFY(set_pos, m_pos)
    }

    std::pair<float, float> ZNodeStatus::get_pos() const
    {
        return m_pos;
    }

} // namespace zeno

#include "GraphModel.h"
#include "uicommon.h"
#include "zassert.h"
#include "variantptr.h"
#include <zeno/utils/api.h>
#include <zeno/core/NodeImpl.h>
#include "model/GraphsTreeModel.h"
#include "model/graphsmanager.h"
#include "parammodel.h"
#include "LinkModel.h"
#include "zenoapplication.h"
#include <zeno/extra/SubnetNode.h>
#include <zeno/core/Assets.h>
#include <zeno/core/data.h>
#include <zeno/utils/helper.h>
#include "util/uihelper.h"
#include "util/jsonhelper.h"
#include "widgets/ztimeline.h"
#include "zenomainwindow.h"
#include "viewport/displaywidget.h"
#include "declmetatype.h"
#include "command.h"
#include <zeno/core/Graph.h>
#include <zeno/core/INodeClass.h>
#include <zeno/core/INodeImpl.h>
#include <zeno/utils/inputoutput_wrapper.h>


static void triggerView(const QString& nodepath, bool bView) {
    zeno::render_reload_info info;
    info.policy = zeno::Reload_ToggleView;
    info.current_ui_graph;  //由于这是在ui下直接点击view，因此一般都是当前图（api的情况暂不考虑）

    zeno::render_update_info update;
    update.reason = bView ? zeno::Update_View : zeno::Update_Remove;
    update.uuidpath_node_objkey = nodepath.toStdString();
    if (update.reason == zeno::Update_Remove)
    {
        //删除节点和绘制清除是异步执行，故不能用节点数据，必须要导出删除object(或者是list/dict下所有objs）的信息
        auto& sess = zeno::getSession();
        auto spNode = sess.getNodeByUuidPath(update.uuidpath_node_objkey);
        assert(spNode);
        zeno::zany spObject = spNode->get_default_output_object();
        if (spObject) {
            update.remove_objs = zeno::get_obj_paths(spObject.get());
        }
    }
    info.objs.push_back(update);

    const auto& views = zenoApp->getMainWindow()->viewports();
    for (DisplayWidget* view : views) {
        view->reload(info);
    }
}


struct NodeItem : public QObject
{
    Q_OBJECT

public:
    //temp cached data for spNode->...
    QString name;
    QString dispName;
    QString dispIcon;
    zeno::ObjPath uuidPath;
    QString cls;
    QPointF pos;

    std::string m_cbSetPos;
    std::string m_cbSetView;
    std::string m_cbSetByPass;
    //for DopnetWork
    std::string m_cbFrameCached;
    std::string m_cbFrameRemoved;

    zeno::NodeImpl* m_wpNode = nullptr;
    ParamsModel* params = nullptr;
    bool bView = false;
    bool bByPass = false;
    bool bCollasped = false;
    NodeState runState;
    zeno::NodeUIStyle uistyle;

    //for subgraph, but not include assets:
    std::optional<GraphModel*> optSubgraph;

    NodeItem(QObject* parent);
    ~NodeItem();
    void init(GraphModel* pGraphM, zeno::NodeImpl* spNode);
    QString getName() {
        return name;
    }

private:
    void unregister();
};

NodeItem::NodeItem(QObject* parent) : QObject(parent)
{
}

NodeItem::~NodeItem()
{
    unregister();
}

void NodeItem::unregister()
{
    if (auto spNode = m_wpNode)
    {
        bool ret = spNode->unregister_set_pos(m_cbSetPos);
        ZASSERT_EXIT(ret);
        ret = spNode->unregister_set_view(m_cbSetView);
        ZASSERT_EXIT(ret);
        ret = spNode->unregister_set_mute(m_cbSetByPass);
        ZASSERT_EXIT(ret);
        //DopNetwork
        if (auto subnetnode = dynamic_cast<zeno::DopNetwork*>(spNode))
        {
            bool ret = subnetnode->unregister_dopnetworkFrameRemoved(m_cbFrameRemoved);
            ZASSERT_EXIT(ret);
            ret = subnetnode->unregister_dopnetworkFrameCached(m_cbFrameCached);
            ZASSERT_EXIT(ret);
        }
    }
    m_cbSetPos = "";
    m_cbSetView = "";
}

void NodeItem::init(GraphModel* pGraphM, zeno::NodeImpl* spNode)
{
    this->m_wpNode = spNode;

    m_cbSetPos = spNode->register_set_pos([=](std::pair<float, float> pos) {
        this->pos = { pos.first, pos.second };  //update the cache
        QModelIndex idx = pGraphM->indexFromName(this->name);
        emit pGraphM->dataChanged(idx, idx, QVector<int>{ QtRole::ROLE_OBJPOS });
    });

    m_cbSetView = spNode->register_set_view([=](bool bView) {
        this->bView = bView;
        QModelIndex idx = pGraphM->indexFromName(this->name);
        emit pGraphM->dataChanged(idx, idx, QVector<int>{ QtRole::ROLE_NODE_ISVIEW });

        //直接就在这里触发绘制更新，不再往外传递了
        //不过有一种例外，就是当前打view会触发计算的话，后续由计算触发绘制更新就可以了
        auto& session = zeno::getSession();
        if (bView && session.is_auto_run() && idx.data(QtRole::ROLE_NODE_DIRTY).toBool()) {
            return;
        }
        const QString& nodepath = idx.data(QtRole::ROLE_NODE_UUID_PATH).toString();
        triggerView(nodepath, bView);
    });

    m_cbSetByPass = spNode->register_set_mute([=](bool bypass) {
        this->bByPass = bypass;
        QModelIndex idx = pGraphM->indexFromName(this->name);
        emit pGraphM->dataChanged(idx, idx, QVector<int>{ QtRole::ROLE_NODE_BYPASS });
    });

    this->params = new ParamsModel(spNode, this);
    this->name = QString::fromStdString(spNode->get_name());
    this->cls = QString::fromStdString(spNode->get_nodecls());
    this->dispName = QString::fromStdString(spNode->get_show_name());
    this->dispIcon = QString::fromStdString(spNode->get_show_icon());
    this->bView = spNode->is_view();
    this->bByPass = spNode->is_mute();
    this->runState.bDirty = spNode->is_dirty();
    this->runState.runstatus = spNode->get_run_status();
    auto pair = spNode->get_pos();
    this->pos = QPointF(pair.first, pair.second);
    this->uuidPath = spNode->get_uuid_path();
    this->uistyle = spNode->coreNode()->export_customui().uistyle;

    setProperty("uuid-path", QString::fromStdString(uuidPath));
    if (auto subnetnode = dynamic_cast<zeno::SubnetNode*>(spNode->coreNode()))
    {
        GraphModel* parentM = qobject_cast<GraphModel*>(this->parent());
        std::string const& subnetpath = spNode->get_path();
        auto pModel = new GraphModel(subnetpath, false, parentM->treeModel(), this);
        bool bAssets = subnetnode->subgraph->isAssets();
        if (bAssets) {
            if (!subnetnode->m_pAdapter->m_pImpl->in_asset_file())
                pModel->setLocked(true);
        }
        this->optSubgraph = pModel;
    }
    //DopNetwork
    if (auto subnetnode = dynamic_cast<zeno::DopNetwork*>(spNode))
    {
        m_cbFrameRemoved = subnetnode->register_dopnetworkFrameRemoved([](int frame) {
            ZenoMainWindow* mainWin = zenoApp->getMainWindow();
            ZASSERT_EXIT(mainWin);
            ZTimeline* timeline = mainWin->timeline();
            ZASSERT_EXIT(timeline);
            timeline->updateDopnetworkFrameRemoved(frame);
        });
        m_cbFrameCached = subnetnode->register_dopnetworkFrameCached([](int frame) {
            ZenoMainWindow* mainWin = zenoApp->getMainWindow();
            ZASSERT_EXIT(mainWin);
            ZTimeline* timeline = mainWin->timeline();
            ZASSERT_EXIT(timeline);
            timeline->updateDopnetworkFrameCached(frame);
        });
    }
}

#include "GraphModel.moc"

struct GraphMImpl : QObject
{
    GraphMImpl(zeno::Graph* spGraph, GraphModel* parent)
        : QObject(parent)
        , m_wpCoreGraph(spGraph)
        , m_parentM(parent)
    {
    }

    zeno::Graph* m_wpCoreGraph;
    GraphModel* m_parentM;
};

GraphModel::GraphModel(std::string const& asset_or_graphpath, bool bAsset, GraphsTreeModel* pTree, QObject* parent)
    : QAbstractListModel(parent)
    , m_pTree(pTree)
{
    std::shared_ptr<zeno::Graph> spGraph;
    if (bAsset) {
        std::shared_ptr<zeno::AssetsMgr> assets = zeno::getSession().assets;
        spGraph = assets->getAssetGraph(asset_or_graphpath, true);
    }
    else {
        spGraph = zeno::getSession().getGraphByPath(asset_or_graphpath);
    }

    m_impl = new GraphMImpl(spGraph.get(), this);

    m_graphName = QString::fromStdString(spGraph->getName());
    m_undoRedoStack = m_graphName == "main" || zeno::getSession().assets->isAssetGraph(spGraph.get()) ? new QUndoStack(this) : nullptr;
    m_linkModel = new LinkModel(this);
    registerCoreNotify();

    std::map<std::string, zeno::NodeImpl*> nodes;
    nodes = spGraph->getNodes();

    for (auto& [name, node] : nodes) {
        _appendNode(node);
    }

    _initLink();

    if (m_pTree) {
        connect(this, SIGNAL(rowsInserted(const QModelIndex&, int, int)),
            m_pTree, SLOT(onGraphRowsInserted(const QModelIndex&, int, int)));
        connect(this, SIGNAL(rowsAboutToBeRemoved(const QModelIndex&, int, int)),
            m_pTree, SLOT(onGraphRowsAboutToBeRemoved(const QModelIndex&, int, int)));
        connect(this, SIGNAL(rowsRemoved(const QModelIndex&, int, int)),
            m_pTree, SLOT(onGraphRowsRemoved(const QModelIndex&, int, int)));
        connect(this, SIGNAL(nameUpdated(const QModelIndex&, const QString&)),
            m_pTree, SLOT(onNameUpdated(const QModelIndex&, const QString&)));
    }
}

void GraphModel::registerCoreNotify()
{
    auto coreGraph = m_impl->m_wpCoreGraph;
    ZASSERT_EXIT(coreGraph);

    m_cbCreateNode = coreGraph->register_createNode([&](const std::string& name, zeno::NodeImpl* spNode) {
        _appendNode(spNode);
    });

    m_cbRemoveNode = coreGraph->register_removeNode([&](const std::string& name) {
        //before core remove
        QString qName = QString::fromStdString(name);
        ZASSERT_EXIT(m_name2uuid.find(qName) != m_name2uuid.end(), false);
        QString uuid = m_name2uuid[qName];
        ZASSERT_EXIT(m_uuid2Row.find(uuid) != m_uuid2Row.end(), false);
        int row = m_uuid2Row[uuid];
        NodeItem* pItem = m_nodes[uuid];
        if (pItem && pItem->bView) {
            const QString& nodepath = QString::fromStdString(pItem->uuidPath);
            triggerView(nodepath, false);
        }
        removeRow(row);
        zenoApp->graphsManager()->currentModel()->markDirty(true);
    });

    m_cbAddLink = coreGraph->register_addLink([&](zeno::EdgeInfo edge) -> bool {
        _addLink_callback(edge);
        return true;
    });

    m_cbRenameNode = coreGraph->register_updateNodeName([&](std::string oldname, std::string newname) {
        const QString& oldName = QString::fromStdString(oldname);
        const QString& newName = QString::fromStdString(newname);
        _updateName(oldName, newName);
    });

    m_cbRemoveLink = coreGraph->register_removeLink([&](zeno::EdgeInfo edge) -> bool {
        return _removeLink(edge);
    });

    m_cbClearGraph = coreGraph->register_clear([&]() {
        //begin clear.
        _clear();
    });
}

void GraphModel::unRegisterCoreNotify()
{
    if (auto coreGraph = m_impl->m_wpCoreGraph)
    {
        bool ret = coreGraph->unregister_createNode(m_cbCreateNode);
        ZASSERT_EXIT(ret);
        ret = coreGraph->unregister_removeNode(m_cbRemoveNode);
        ZASSERT_EXIT(ret);
        ret = coreGraph->unregister_addLink(m_cbAddLink);
        ZASSERT_EXIT(ret);
        ret = coreGraph->unregister_removeLink(m_cbRemoveLink);
        ZASSERT_EXIT(ret);
        ret = coreGraph->unregister_updateNodeName(m_cbRenameNode);
        ZASSERT_EXIT(ret);
        ret = coreGraph->unregister_clear(m_cbClearGraph);
        ZASSERT_EXIT(ret);
    }
    m_cbCreateNode = "";
    m_cbCreateNode = "";
    m_cbAddLink = "";
    m_cbRemoveLink = "";
    m_cbRenameNode = "";
    m_cbClearGraph = "";
}

GraphModel::~GraphModel()
{
    unRegisterCoreNotify();
}

void GraphModel::clear()
{
    auto spGraph = m_impl->m_wpCoreGraph;
    ZASSERT_EXIT(spGraph);
    spGraph->clear();
}

int GraphModel::indexFromId(const QString& name) const
{
    if (m_name2uuid.find(name) == m_name2uuid.end())
        return -1;

    QString uuid = m_name2uuid[name];
    if (m_uuid2Row.find(uuid) == m_uuid2Row.end())
        return -1;
    return m_uuid2Row[uuid];
}

QModelIndex GraphModel::indexFromName(const QString& name) const {
    int row = indexFromId(name);
    if (row == -1) {
        return QModelIndex();
    }
    return createIndex(row, 0);
}

std::set<std::string> GraphModel::getViewNodePath() const {
    auto viewnodenames = m_impl->m_wpCoreGraph->get_viewnodes();
    std::set<std::string> nodes;
    for (auto name : viewnodenames) {
        QModelIndex idx = indexFromName(QString::fromStdString(name));
        const QString& nodepath = idx.data(QtRole::ROLE_NODE_UUID_PATH).toString();
        nodes.insert(nodepath.toStdString());
    }
    return nodes;
}

void GraphModel::addLink(const QString& fromNodeStr, const QString& fromParamStr,
    const QString& toNodeStr, const QString& toParamStr)
{
    zeno::EdgeInfo link;
    link.inNode = toNodeStr.toStdString();
    link.inParam = toParamStr.toStdString();
    link.outNode = fromNodeStr.toStdString();
    link.outParam = fromParamStr.toStdString();
    _addLink_apicall(link, true);
}

void GraphModel::addLink(const zeno::EdgeInfo& link)
{
    _addLink_apicall(link, true);
}

QString GraphModel::name() const
{
    return m_graphName;
}

QStringList GraphModel::path() const
{
    return currentPath();
}

void GraphModel::insertNode(const QString& nodeCls, const QString& cate, const QPointF& pos) {
    zeno::NodeData dat = createNode(nodeCls, cate, pos);
}

QString GraphModel::owner() const
{
    if (auto pItem = qobject_cast<NodeItem*>(parent()))
    {
        auto spNode = pItem->m_wpNode;
        return spNode ? QString::fromStdString(spNode->get_name()) : "";
    }
    else {
        return "main";
    }
}

int GraphModel::rowCount(const QModelIndex& parent) const
{
    return m_nodes.size();
}

QVariant GraphModel::data(const QModelIndex& index, int role) const
{
    NodeItem* item = m_nodes[m_row2uuid[index.row()]];

    switch (role) {
        case Qt::DisplayRole:
        case QtRole::ROLE_NODE_NAME: {
            return item->name;
        }
        case QtRole::ROLE_NODE_DISPLAY_NAME: {
            return item->dispName;
        }
        case QtRole::ROLE_NODE_DISPLAY_ICON: {
            return item->dispIcon;
        }
        case QtRole::ROLE_NODE_UISTYLE: {
            QVariantMap map;
            map["icon"] = QString::fromStdString(item->uistyle.iconResPath);
            map["background"] = QString::fromStdString(item->uistyle.background);
            return map;
        }
        case QtRole::ROLE_NODE_UUID_PATH: {
            return QString::fromStdString(item->uuidPath);
        }
        case QtRole::ROLE_CLASS_NAME: {
            return item->cls;
        }
        case QtRole::ROLE_OBJPOS: {
            return item->pos;
        }
        case QtRole::ROLE_PARAMS:
        {
            return QVariantPtr<ParamsModel>::asVariant(item->params);
        }
        case QtRole::ROLE_SUBGRAPH:
        {
            if (item->optSubgraph.has_value())
                return QVariant::fromValue(item->optSubgraph.value());
            else
                return QVariant();  
        }
        case QtRole::ROLE_GRAPH:
        {
            return QVariantPtr<GraphModel>::asVariant(const_cast<GraphModel*>(this));
        }
        case QtRole::ROLE_INPUTS:
        {
            if (item->params)
                return QVariant::fromValue(item->params->getInputs());
            return QVariant();
        }
        case QtRole::ROLE_OUTPUTS:
        {
            if (item->params)
                return QVariant::fromValue(item->params->getOutputs());
            return QVariant();
        }
        case QtRole::ROLE_NODEDATA:
        {
            zeno::NodeData data;
            auto spNode = item->m_wpNode;
            ZASSERT_EXIT(spNode, QVariant());
            return QVariant::fromValue(spNode->exportInfo());
        }
        case QtRole::ROLE_NODE_STATUS:
        {
            int options = zeno::None;
            if (item->bView)
                options |= zeno::View;
            if (item->bByPass)
                options |= zeno::Mute;
            return QVariant(options);
        }
        case QtRole::ROLE_OUTPUT_OBJS:
        {
            auto spNode = item->m_wpNode;
            ZASSERT_EXIT(spNode, QVariant());
            zeno::zany spOutObj = spNode->get_default_output_object();
            //如果有多个，只取第一个。
            if (spOutObj) {
                return QVariant::fromValue(spOutObj);
            }
            else {
                return QVariant();
            }
        }
        case QtRole::ROLE_NODE_ISVIEW:
        {
            return item->bView;
        }
        case QtRole::ROLE_NODE_BYPASS:
        {
            return item->bByPass;
        }
        case QtRole::ROLE_NODE_RUN_STATE:
        {
            return QVariant::fromValue(item->runState);
        }
        case QtRole::ROLE_NODE_DIRTY:
        {
            return item->runState.bDirty;
        }
        case QtRole::ROLE_NODETYPE:
        {
            auto spNode = item->m_wpNode;
            auto spSubnetNode = dynamic_cast<zeno::SubnetNode*>(spNode->coreNode());
            if (spSubnetNode) {
                bool bAssets = spSubnetNode->subgraph->isAssets();
                if (bAssets) {
                    if (spSubnetNode->m_pAdapter->m_pImpl->in_asset_file())
                        return zeno::Node_AssetReference;
                    else
                        return zeno::Node_AssetInstance;
                }
                return zeno::Node_SubgraphNode;
            }
            if (spNode && spNode->get_nodecls() == "Group")
                return zeno::Node_Group;
            return zeno::Node_Normal;
        }
        case QtRole::ROLE_NODE_CATEGORY:
        {
            auto spNode = item->m_wpNode;
            if (spNode->nodeClass)
            {
                return QString::fromStdString(spNode->nodeClass->m_customui.category);
            }
            else
            {
                return "";
            }
        }
        case QtRole::ROLE_OBJPATH:
        {
            auto spNode = item->m_wpNode;
            if (spNode) {
                return QString::fromStdString(spNode->get_path());
            }
            //QStringList path = currentPath();
            //path.append(item->name);
            return "";
        }
        case QtRole::ROLE_COLLASPED:
        {
            return item->bCollasped;
        }
        case QtRole::ROLE_KEYFRAMES: 
        {
            QVector<int> keys;
            for (const zeno::ParamPrimitive& info : item->params->getInputs()) {
                if (!info.defl.has_value()) {
                    continue;
                }
                const QVariant& value = QVariant::fromValue(info.defl);

                bool bValid = false;
                zeno::CurvesData curves = UiHelper::getCurvesFromQVar(value, &bValid);
                if (curves.empty() || !bValid) {
                    continue;
                }

                for (auto& [_, curve] : curves.keys) {
                    QVector<int> cpbases;
                    for (auto xval : curve.cpbases) {
                        cpbases << xval;
                    }
                    keys << cpbases;
                }
            }
            keys.erase(std::unique(keys.begin(), keys.end()), keys.end());
            return QVariant::fromValue(keys);
        }
        //Dopnetwork
        case QtRole::ROLE_DOPNETWORK_ENABLECACHE: {
            if (auto spNode = item->m_wpNode) {
                if (auto spDop = dynamic_cast<zeno::DopNetwork*>(spNode)) {
                    return spDop->m_bEnableCache;
                }
            }
        }
        case QtRole::ROLE_DOPNETWORK_CACHETODISK: {
            if (auto spNode = item->m_wpNode) {
                if (auto spDop = dynamic_cast<zeno::DopNetwork*>(spNode)) {
                    return spDop->m_bAllowCacheToDisk;
                }
            }
        }
        case QtRole::ROLE_DOPNETWORK_MAXMEM: {
            if (auto spNode = item->m_wpNode) {
                if (auto spDop = dynamic_cast<zeno::DopNetwork*>(spNode)) {
                    return spDop->m_maxCacheMemoryMB;
                }
            }
        }
        case QtRole::ROLE_DOPNETWORK_MEM: {
            if (auto spNode = item->m_wpNode) {
                if (auto spDop = dynamic_cast<zeno::DopNetwork*>(spNode)) {
                    return spDop->m_currCacheMemoryMB;
                }
            }
        }
        default:
            return QVariant();
    }
}

bool GraphModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    NodeItem* item = m_nodes[m_row2uuid[index.row()]];

    switch (role) {
        case QtRole::ROLE_CLASS_NAME: {
            //TODO: rename by core graph
            emit dataChanged(index, index, QVector<int>{role});
            return true;
        }
        case QtRole::ROLE_NODE_NAME: {
            updateNodeName(index, value.toString());
            return true;
        }
        case QtRole::ROLE_OBJPOS:
        {
            auto spNode = item->m_wpNode;
            if (value.type() == QVariant::PointF) {
                QPointF pos = value.toPoint();
                spNode->set_pos({ pos.x(), pos.y() });
            }
            else {
                QVariantList lst = value.toList();
                if (lst.size() != 2)
                    return false;
                spNode->set_pos({ lst[0].toFloat(), lst[1].toFloat() });
            }
            return true;
        }
        case QtRole::ROLE_COLLASPED:
        {
            item->bCollasped = value.toBool();
            emit dataChanged(index, index, QVector<int>{role});
            return true;
        }
        case QtRole::ROLE_NODE_RUN_STATE:
        {
            item->runState = value.value<NodeState>();
            emit dataChanged(index, index, QVector<int>{role});
            return true;
        }
        case QtRole::ROLE_NODE_STATUS:
        {
            setView(index, value.toInt() & zeno::View);
            // setMute();
            break;
        }
        case QtRole::ROLE_NODE_ISVIEW:
        {
            setView(index, value.toBool());
            break;
        }
        case QtRole::ROLE_NODE_BYPASS:
        {
            setMute(index, value.toBool());
            break;
        }
        case QtRole::ROLE_INPUTS:
        {
            PARAMS_INFO paramsInfo = value.value<PARAMS_INFO>();
            for (auto&[key, param] : paramsInfo)
            {
                QModelIndex idx = item->params->paramIdx(key, true);
                item->params->setData(idx, QVariant::fromValue(param.defl), QtRole::ROLE_PARAM_VALUE);
            }
            return true;
        }
        //Dopnetwork
        case QtRole::ROLE_DOPNETWORK_ENABLECACHE: {
            if (auto spNode = item->m_wpNode) {
                if (auto spDop = dynamic_cast<zeno::DopNetwork*>(spNode)) {
                    spDop->setEnableCache(value.toBool());
                }
            }
        }
        case QtRole::ROLE_DOPNETWORK_CACHETODISK: {
            if (auto spNode = item->m_wpNode) {
                if (auto spDop = dynamic_cast<zeno::DopNetwork*>(spNode)) {
                    spDop->setAllowCacheToDisk(value.toBool());
                }
            }
        }
        case QtRole::ROLE_DOPNETWORK_MAXMEM: {
            if (auto spNode = item->m_wpNode) {
                if (auto spDop = dynamic_cast<zeno::DopNetwork*>(spNode)) {
                    spDop->setMaxCacheMemoryMB(value.toInt());
                }
            }
        }
        case QtRole::ROLE_DOPNETWORK_MEM: {
            if (auto spNode = item->m_wpNode) {
                if (auto spDop = dynamic_cast<zeno::DopNetwork*>(spNode)) {
                    spDop->setCurrCacheMemoryMB(value.toInt());
                }
            }
        }
    }
    return false;
}

QModelIndexList GraphModel::match(const QModelIndex& start, int role,
    const QVariant& value, int hits,
    Qt::MatchFlags flags) const
{
    QModelIndexList result;
    if (role == QtRole::ROLE_CLASS_NAME) {
        auto spGraph = m_impl->m_wpCoreGraph;
        ZASSERT_EXIT(spGraph, result);
        std::string content = value.toString().toStdString();
        auto results = spGraph->searchByClass(content);
        for (std::string node : results) {
            QModelIndex nodeIdx = indexFromName(QString::fromStdString(node));
            result.append(nodeIdx);
        }
    }
    return result;
}

QList<SEARCH_RESULT> GraphModel::search(const QString& content, SearchType searchType, SearchOpt searchOpts)
{
    QList<SEARCH_RESULT> results;
    if (content.isEmpty())
        return results;

    if (searchType & SEARCH_NODEID) {
        QModelIndexList lst;
        if (searchOpts == SEARCH_MATCH_EXACTLY) {
            QModelIndex idx = indexFromName(content);
            if (idx.isValid())
                lst.append(idx);
        }
        else {
            lst = _base::match(this->index(0, 0), QtRole::ROLE_NODE_NAME, content, -1, Qt::MatchContains);
        }
        if (!lst.isEmpty()) {
            for (const QModelIndex& nodeIdx : lst) {
                SEARCH_RESULT result;
                result.targetIdx = nodeIdx;
                result.subGraph = this;
                result.type = SEARCH_NODEID;
                results.append(result);
            }
        }
        for (auto& subnode : m_subgNodes)
        {
            if (m_name2uuid.find(subnode) == m_name2uuid.end())
                continue;
            NodeItem* pItem = m_nodes[m_name2uuid[subnode]];
            if (!pItem->optSubgraph.has_value())
                continue;
            QList<SEARCH_RESULT>& subnodeRes = pItem->optSubgraph.value()->search(content, searchType, searchOpts);
            for (auto& res: subnodeRes)
                results.push_back(res);
        }
    }

    //TODO

    return results;
}

QList<SEARCH_RESULT> GraphModel::searchByUuidPath(const zeno::ObjPath& uuidPath)
{
    QList<SEARCH_RESULT> results;
    if (uuidPath.empty())
        return results;

    SEARCH_RESULT result;
    result.targetIdx = indexFromUuidPath(uuidPath);
    result.subGraph = getGraphByPath(uuidPath2ObjPath(uuidPath));
    result.type = SEARCH_NODEID;
    results.append(result);
    return results;
}

QStringList GraphModel::uuidPath2ObjPath(const zeno::ObjPath& uuidPath)
{
    QStringList res;
    zeno::ObjPath tmp = uuidPath;
    if (tmp.empty())
        return res;

    int idx = tmp.find("/");
    auto uuid = tmp.substr(0, idx);
    auto it = m_nodes.find(QString::fromStdString(uuid));
    if (it == m_nodes.end()) {
        NodeItem* pItem = it.value();
        res.append(pItem->getName());

        if (idx >= 0)
            tmp = tmp.substr(idx+1, tmp.size() - idx);

        if (pItem->optSubgraph.has_value())
            res.append(pItem->optSubgraph.value()->uuidPath2ObjPath(tmp));
    }

    return res;
}

QModelIndex GraphModel::indexFromUuidPath(const zeno::ObjPath& uuidPath)
{
    if (uuidPath.empty())
        return QModelIndex();

    int idx = uuidPath.find("/");
    const QString& uuid = QString::fromStdString(uuidPath.substr(0, idx));
    if (m_nodes.find(uuid) != m_nodes.end()) {
        NodeItem* pItem = m_nodes[uuid];
        zeno::ObjPath _path = uuidPath;
        if (idx < 0) {
            return createIndex(m_uuid2Row[uuid], 0, nullptr);
        }
        else if (pItem->optSubgraph.has_value()) {
            _path = uuidPath.substr(idx + 1, uuidPath.size() - idx);
            return pItem->optSubgraph.value()->indexFromUuidPath(_path);
        }
    }
    return QModelIndex();
}

GraphModel* GraphModel::getGraphByPath(const QStringList& objPath)
{
    QStringList items = objPath;
    if (items.empty())
        return this;

    QString item = items[0];

    if (m_name2uuid.find(item) == m_name2uuid.end()) {
        return nullptr;
    }

    QString uuid = m_name2uuid[item];
    auto it = m_nodes.find(uuid);
    if (it == m_nodes.end()) {
        return nullptr;
    }

    NodeItem* pItem = it.value();
    items.removeAt(0);

    if (items.isEmpty())
    {
        if (pItem->optSubgraph.has_value())
        {
            return pItem->optSubgraph.value();
        }
        else
        {
            return this;
        }
    }
    ZASSERT_EXIT(pItem->optSubgraph.has_value(), nullptr);
    return pItem->optSubgraph.value()->getGraphByPath(items);
}

QStringList GraphModel::currentPath() const
{
    QStringList path;
    NodeItem* pNode = qobject_cast<NodeItem*>(this->parent());
    if (!pNode)
        return { this->m_graphName };

    GraphModel* pGraphM = nullptr;
    while (pNode) {
        path.push_front(pNode->name);
        pGraphM = qobject_cast<GraphModel*>(pNode->parent());
        ZASSERT_EXIT(pGraphM, {});
        pNode = qobject_cast<NodeItem*>(pGraphM->parent());
    }
    path.push_front(pGraphM->name());
    return path;
}

void GraphModel::undo()
{
    zeno::getSession().beginApiCall();
    zeno::scope_exit scope([=]() { zeno::getSession().endApiCall(); });
    if (m_undoRedoStack.has_value() && m_undoRedoStack.value())
        m_undoRedoStack.value()->undo();
}

void GraphModel::redo()
{
    zeno::getSession().beginApiCall();
    zeno::scope_exit scope([=]() { zeno::getSession().endApiCall(); });
    if (m_undoRedoStack.has_value() && m_undoRedoStack.value())
        m_undoRedoStack.value()->redo();
}

void GraphModel::pushToplevelStack(QUndoCommand* cmd)
{
    if (m_undoRedoStack.has_value() && m_undoRedoStack.value())
        m_undoRedoStack.value()->push(cmd);
}

void GraphModel::beginMacro(const QString& name)
{
    auto curpath = currentPath();
    if (curpath.size() > 1)   //不是顶层graph，则调用顶层graph
    {
        if (GraphModel* topLevelGraph = getTopLevelGraph(curpath))
            topLevelGraph->beginMacro(name);
    }
    else {
        if (m_undoRedoStack.has_value() && m_undoRedoStack.value())
            m_undoRedoStack.value()->beginMacro(name);
        zeno::getSession().beginApiCall();
    }
}

void GraphModel::endMacro()
{
    auto curpath = currentPath();
    if (curpath.size() > 1)   //不是顶层graph，则调用顶层graph
    {
        if (GraphModel* topLevelGraph = getTopLevelGraph(curpath))
            topLevelGraph->endMacro();
    }
    else {
        if (m_undoRedoStack.has_value() && m_undoRedoStack.value())
            m_undoRedoStack.value()->endMacro();
        zeno::getSession().endApiCall();
    }
}

void GraphModel::_initLink()
{
    for (auto item : m_nodes)
    {
        auto spNode = item->m_wpNode;
        ZASSERT_EXIT(spNode);
        zeno::NodeData nodedata = spNode->exportInfo();
        //objects links init
        for (auto param : nodedata.customUi.inputObjs) {
            for (auto link : param.links) {
                link.bObjLink = true;
                _addLink_callback(link);
            }
        }
        //primitives links init
        for (auto tab : nodedata.customUi.inputPrims) {
            for (auto group : tab.groups) {
                for (auto param : group.params) {
                    for (auto link : param.links) {
                        link.bObjLink = false;
                        _addLink_callback(link);
                    }
                }
            }
        }
    }
}

void GraphModel::_addLink_callback(const zeno::EdgeInfo link)
{
    QModelIndex from, to;

    QString outNode = QString::fromStdString(link.outNode);
    QString outParam = QString::fromStdString(link.outParam);
    QString outKey = QString::fromStdString(link.outKey);
    QString inNode = QString::fromStdString(link.inNode);
    QString inParam = QString::fromStdString(link.inParam);
    QString inKey = QString::fromStdString(link.inKey);

    if (m_name2uuid.find(outNode) == m_name2uuid.end() ||
        m_name2uuid.find(inNode) == m_name2uuid.end())
        return;

    ParamsModel* fromParams = m_nodes[m_name2uuid[outNode]]->params;
    ParamsModel* toParams = m_nodes[m_name2uuid[inNode]]->params;

    from = fromParams->paramIdx(outParam, false);
    to = toParams->paramIdx(inParam, true);
    
    if (from.isValid() && to.isValid())
    {
        //notify ui to create dict key slot.
        if (!link.inKey.empty())
            emit toParams->linkAboutToBeInserted(link);
        if (!link.outKey.empty()) {
            if (zenoApp->graphsManager()->isInitializing())
                emit fromParams->linkAboutToBeInserted(link);
        }

        QModelIndex linkIdx = m_linkModel->addLink(from, outKey, to, inKey, link.bObjLink);
        fromParams->addLink(from, linkIdx);
        toParams->addLink(to, linkIdx);
    }

    zenoApp->graphsManager()->currentModel()->markDirty(true);
}

bool GraphModel::removeLink(
    const QString& outNode,
    const QString& outParam,
    const QString& inNode,
    const QString& inParam,
    const QString& outKey,
    const QString& inKey)
{
    zeno::EdgeInfo edge;
    edge.inKey = inKey.toStdString();
    edge.inParam = inParam.toStdString();
    edge.inNode = inNode.toStdString();
    edge.outKey = outKey.toStdString();
    edge.outParam = outParam.toStdString();
    edge.outNode = outNode.toStdString();
    removeLink(edge);
    return true;
}

void GraphModel::removeLink(const QModelIndex& linkIdx)
{
    zeno::EdgeInfo edge = linkIdx.data(QtRole::ROLE_LINK_INFO).value<zeno::EdgeInfo>();
    removeLink(edge);
}

void GraphModel::removeLink(const zeno::EdgeInfo& link)
{
    _removeLinkImpl(link, true);
}

bool GraphModel::updateLink(const QModelIndex& linkIdx, bool bInput, const QString& oldkey, const QString& newkey)
{
    auto spGraph = m_impl->m_wpCoreGraph;
    ZASSERT_EXIT(spGraph, false);
    zeno::EdgeInfo edge = linkIdx.data(QtRole::ROLE_LINK_INFO).value<zeno::EdgeInfo>();
    bool ret = spGraph->updateLink(edge, bInput, oldkey.toStdString(), newkey.toStdString());
    if (!ret)
        return ret;

    QAbstractItemModel* pModel = const_cast<QAbstractItemModel*>(linkIdx.model());
    LinkModel* linksM = qobject_cast<LinkModel*>(pModel);
    linksM->setData(linkIdx, newkey, QtRole::ROLE_LINK_INKEY);
}

void GraphModel::moveUpLinkKey(const QModelIndex& linkIdx, bool bInput, const std::string& keyName)
{
    auto spGraph = m_impl->m_wpCoreGraph;
    ZASSERT_EXIT(spGraph);
    zeno::EdgeInfo edge = linkIdx.data(QtRole::ROLE_LINK_INFO).value<zeno::EdgeInfo>();
    spGraph->moveUpLinkKey(edge, bInput, keyName);
}

bool GraphModel::_removeLink(const zeno::EdgeInfo& edge)
{
    QString outNode = QString::fromStdString(edge.outNode);
    QString inNode = QString::fromStdString(edge.inNode);
    QString outParam = QString::fromStdString(edge.outParam);
    QString inParam = QString::fromStdString(edge.inParam);

    ZASSERT_EXIT(m_name2uuid.find(outNode) != m_name2uuid.end(), false);
    ZASSERT_EXIT(m_name2uuid.find(inNode) != m_name2uuid.end(), false);

    QModelIndex from, to;

    ParamsModel* fromParams = m_nodes[m_name2uuid[outNode]]->params;
    ParamsModel* toParams = m_nodes[m_name2uuid[inNode]]->params;

    from = fromParams->paramIdx(outParam, false);
    to = toParams->paramIdx(inParam, true);
    if (from.isValid() && to.isValid())
    {
        emit toParams->linkAboutToBeRemoved(edge);
        QModelIndex linkIdx = fromParams->removeOneLink(from, edge);
        QModelIndex linkIdx2 = toParams->removeOneLink(to, edge);
        ZASSERT_EXIT(linkIdx == linkIdx2, false);
        if (linkIdx.isValid())
            m_linkModel->removeRow(linkIdx.row());
    }
    zenoApp->graphsManager()->currentModel()->markDirty(true);
    return true;
}

void GraphModel::_updateName(const QString& oldName, const QString& newName)
{
    ZASSERT_EXIT(oldName != newName);

    m_name2uuid[newName] = m_name2uuid[oldName];

    QString uuid = m_name2uuid[newName];
    auto& item = m_nodes[uuid];
    item->name = newName;   //update cache.

    if (m_subgNodes.find(oldName) != m_subgNodes.end()) {
        m_subgNodes.remove(oldName);
        m_subgNodes.insert(newName);
    }

    int row = m_uuid2Row[uuid];
    QModelIndex idx = createIndex(row, 0);
    emit dataChanged(idx, idx, QVector<int>{ QtRole::ROLE_NODE_NAME });
    emit nameUpdated(idx, oldName);
    zenoApp->graphsManager()->currentModel()->markDirty(true);
}

zeno::NodeData GraphModel::createNode(const QString& nodeCls, const QString& cate, const QPointF& pos)
{
    zeno::NodeData nodedata;
    nodedata.cls = nodeCls.toStdString();
    nodedata.uipos = { pos.x(), pos.y() };
    return _createNodeImpl(cate, nodedata, true);
}

void GraphModel::_appendNode(void* _spNode)
{
    zeno::NodeImpl* pNodeImpl = static_cast<zeno::NodeImpl*>(_spNode);
    int nRows = m_nodes.size();

    beginInsertRows(QModelIndex(), nRows, nRows);

    NodeItem* pItem = new NodeItem(this);
    pItem->init(this, pNodeImpl);
    pItem->params->setNodeIdx(createIndex(nRows, 0));

    const QString& name = QString::fromStdString(pNodeImpl->get_name());
    const QString& uuid = QString::fromStdString(pNodeImpl->get_uuid());

    if (pItem->optSubgraph.has_value())
        m_subgNodes.insert(name);

    m_row2uuid[nRows] = uuid;
    m_uuid2Row[uuid] = nRows;
    m_nodes.insert(uuid, pItem);
    m_name2uuid.insert(name, uuid);

    endInsertRows();

    zenoApp->graphsManager()->currentModel()->markDirty(true);
}

zeno::NodeData GraphModel::_createNodeImpl(const QString& cate, zeno::NodeData& nodedata, bool endTransaction)
{
    bool bEnableIoProc = zenoApp->graphsManager()->isInitializing() || zenoApp->graphsManager()->isImporting();
    if (bEnableIoProc)
        endTransaction = false;

    if (endTransaction)
    {
        auto currtPath = currentPath();
        AddNodeCommand* pCmd = new AddNodeCommand(cate, nodedata, currtPath);
        if (auto topLevelGraph = getTopLevelGraph(currtPath))
        {
            topLevelGraph->pushToplevelStack(pCmd);
            return pCmd->getNodeData();
        }
        return zeno::NodeData();
    }
    else {
        auto updateInputs = [](zeno::NodeData& nodedata, zeno::INodeImpl* spNode) {
            for (auto& tab : nodedata.customUi.inputPrims)
            {
                for (auto& group : tab.groups)
                {
                    for (auto& param : group.params)
                    {
                        spNode->m_pImpl->update_param(param.name, param.defl);
                    }
                }
            }
        };

        auto spGraph = m_impl->m_wpCoreGraph;
        if (!spGraph)
            return zeno::NodeData();

        auto spNode = spGraph->createNode(nodedata.cls, nodedata.name, cate == "assets", nodedata.uipos);
        if (!spNode)
            return zeno::NodeData();

        zeno::NodeData node;

        if (zeno::isDerivedFromSubnetNodeName(nodedata.cls) || cate == "assets") {
            QString nodeName = QString::fromStdString(spNode->m_pImpl->get_name());
            QString uuid = m_name2uuid[nodeName];
            ZASSERT_EXIT(m_nodes.find(uuid) != m_nodes.end(), zeno::NodeData());
            auto paramsM = m_nodes[uuid]->params;

            if (auto subnetNode = dynamic_cast<zeno::SubnetNode*>(spNode->m_pImpl->coreNode())) {
                //create input/output in subnet
                if (cate == "assets")
                {
                    const auto asset = zeno::getSession().assets->getAsset(nodedata.cls);
                    nodedata.customUi = asset.m_customui;
                }
                //zeno::ParamsUpdateInfo updateInfo;
                //zeno::parseUpdateInfo(nodedata.customUi, updateInfo);
                //paramsM->resetCustomUi(nodedata.customUi);
                //paramsM->batchModifyParams(updateInfo, true);

                if (nodedata.subgraph.has_value())
                {
                    for (auto& [name, nodedata] : nodedata.subgraph.value().nodes)
                    {
                        if (zeno::isDerivedFromSubnetNodeName(nodedata.cls)) {   //if is subnet, create recursively
                            QStringList cur = currentPath();
                            cur.append(QString::fromStdString(spNode->m_pImpl->get_name()));
                            GraphModel* model = zenoApp->graphsManager()->getGraph(cur);
                            if (model)
                                model->_createNodeImpl(cate, nodedata, false);
                        }
                        else if (nodedata.cls == "SubInput" || nodedata.cls == "SubOutput") {   //dont create, just update subinput/output pos
                            auto ioNode = subnetNode->subgraph->getNode(name);
                            if (ioNode)
                                ioNode->set_pos(nodedata.uipos);
                        }
                        else if (nodedata.asset.has_value()) {  //if is asset
                            spNode = subnetNode->subgraph->createNode(nodedata.cls, name, true, {nodedata.uipos.first, nodedata.uipos.second});
                            if (spNode)
                                updateInputs(nodedata, spNode);
                        }
                        else {
                            spNode = subnetNode->subgraph->createNode(nodedata.cls, name, false, {nodedata.uipos.first, nodedata.uipos.second});
                            if (spNode)
                                updateInputs(nodedata, spNode);
                        }
                    }
                    for (zeno::EdgeInfo oldLink : nodedata.subgraph.value().links) {
                        subnetNode->subgraph->addLink(oldLink);
                    }
                }
                updateInputs(nodedata, spNode);
                node = spNode->m_pImpl->exportInfo();
            }
        }
        else {
            updateInputs(nodedata, spNode);
            node = spNode->m_pImpl->exportInfo();
        }

        return node;
    }
}

bool GraphModel::_removeNodeImpl(const QString& name, bool endTransaction)
{
    bool bEnableIoProc = zenoApp->graphsManager()->isInitializing() || zenoApp->graphsManager()->isImporting();
    if (bEnableIoProc)
        endTransaction = false;

    if (endTransaction)
    {
        if (m_name2uuid.find(name) != m_name2uuid.end() && m_nodes.find(m_name2uuid[name]) != m_nodes.end())
        {
            auto spNode = m_nodes[m_name2uuid[name]]->m_wpNode;
            if (spNode)
            {
                auto nodedata = spNode->exportInfo();
                auto currtPath = currentPath();
                RemoveNodeCommand* pCmd = new RemoveNodeCommand(nodedata, currtPath);
                if (auto topLevelGraph = getTopLevelGraph(currtPath))
                {
                    topLevelGraph->pushToplevelStack(pCmd);
                    return true;
                }
                //m_undoRedoStack->push(pCmd);
            }
        }
        return false;
    }
    else {
        //remove all related links
        NodeItem* item = m_nodes[m_name2uuid[name]];
        if (item)
        {
            PARAMS_INFO ioParams = item->params->getInputs();
            ioParams.insert(item->params->getOutputs());
            for (zeno::ParamPrimitive& paramInfo : ioParams)
            {
                for (zeno::EdgeInfo& edge: paramInfo.links)
                {
                    auto currtPath = currentPath();
                    LinkCommand* pCmd = new LinkCommand(false, edge, currentPath());
                    if (auto topLevelGraph = getTopLevelGraph(currtPath))
                        topLevelGraph->pushToplevelStack(pCmd);
                }
            }
        }

        auto spCoreGraph = m_impl->m_wpCoreGraph;
        ZASSERT_EXIT(spCoreGraph, false);
        if (spCoreGraph)
            return spCoreGraph->removeNode(name.toStdString());
        return false;
    }
}

void GraphModel::_addLink_apicall(const zeno::EdgeInfo& link, bool endTransaction)
{
    bool bEnableIoProc = zenoApp->graphsManager()->isInitializing() || zenoApp->graphsManager()->isImporting();
    if (bEnableIoProc)
        endTransaction = false;

    if (endTransaction)
    {
        LinkCommand* pCmd = new LinkCommand(true, link, currentPath());
        auto currtPath = currentPath();
        if (auto topLevelGraph = getTopLevelGraph(currtPath))
        {
            topLevelGraph->pushToplevelStack(pCmd);
        }
    }
    else {
        m_impl->m_wpCoreGraph->addLink(link);
    }
}

void GraphModel::_removeLinkImpl(const zeno::EdgeInfo& link, bool endTransaction)
{
    bool bEnableIoProc = zenoApp->graphsManager()->isInitializing() || zenoApp->graphsManager()->isImporting();
    if (bEnableIoProc)
        endTransaction = false;

    if (endTransaction)
    {
        LinkCommand* pCmd = new LinkCommand(false, link, currentPath());
        auto currtPath = currentPath();
        if (auto topLevelGraph = getTopLevelGraph(currtPath))
        {
            topLevelGraph->pushToplevelStack(pCmd);
        }
    }
    else {
        //emit to core data.
        m_impl->m_wpCoreGraph->removeLink(link);
    }
}

bool GraphModel::setModelData(const QModelIndex& index, const QVariant& newValue, int role)
{
    zeno::getSession().beginApiCall();
    zeno::scope_exit scope([=]() { zeno::getSession().endApiCall(); });

    const auto& oldVal = index.data(role);
    ModelDataCommand* pcmd = new ModelDataCommand(index, oldVal, newValue, role, currentPath());
    if (auto topLevelGraph = getTopLevelGraph(currentPath()))
    {
        topLevelGraph->pushToplevelStack(pcmd);
    }
    return true;
}

void GraphModel::_setViewImpl(const QModelIndex& idx, bool bOn, bool endTransaction)
{
    
    bool bEnableIoProc = zenoApp->graphsManager()->isInitializing() || zenoApp->graphsManager()->isImporting();
    if (bEnableIoProc)
        endTransaction = false;

    if (endTransaction)
    {
        auto currtPath = currentPath();
        NodeStatusCommand* pCmd = new NodeStatusCommand(true, idx.data(QtRole::ROLE_NODE_NAME).toString(), bOn, currtPath);
        if (auto topLevelGraph = getTopLevelGraph(currtPath))
            topLevelGraph->pushToplevelStack(pCmd);
    }
    else {
        auto spCoreGraph = m_impl->m_wpCoreGraph;
        ZASSERT_EXIT(spCoreGraph);
        NodeItem* item = m_nodes[m_row2uuid[idx.row()]];
        auto spCoreNode = item->m_wpNode;
        ZASSERT_EXIT(spCoreNode);
        spCoreNode->set_view(bOn);
    }
}

void GraphModel::_setMuteImpl(const QModelIndex& idx, bool bOn, bool endTransaction)
{
    bool bEnableIoProc = zenoApp->graphsManager()->isInitializing() || zenoApp->graphsManager()->isImporting();
    if (bEnableIoProc)
        endTransaction = false;

    if (endTransaction)
    {
        auto currtPath = currentPath();
        NodeStatusCommand* pCmd = new NodeStatusCommand(false, idx.data(QtRole::ROLE_NODE_NAME).toString(), bOn, currtPath);
        if (auto topLevelGraph = getTopLevelGraph(currtPath))
            topLevelGraph->pushToplevelStack(pCmd);
    }
    else {
        auto spCoreGraph = m_impl->m_wpCoreGraph;
        ZASSERT_EXIT(spCoreGraph);
        NodeItem* item = m_nodes[m_row2uuid[idx.row()]];
        auto spCoreNode = item->m_wpNode;
        ZASSERT_EXIT(spCoreNode);
        spCoreNode->set_mute(bOn);
    }
}

void GraphModel::appendSubgraphNode(QString name, QString cls, NODE_DESCRIPTOR desc, GraphModel* subgraph, const QPointF& pos)
{
    //TODO:
#if 0
    int nRows = m_nodes.size();
    beginInsertRows(QModelIndex(), nRows, nRows);

    NodeItem* pItem = new NodeItem(this);
    pItem->setParent(this);
    pItem->name = name;
    pItem->name = cls;
    pItem->pos = pos;
    pItem->params = new ParamsModel(desc, pItem);
    pItem->pSubgraph = subgraph;
    subgraph->setParent(pItem);

    m_row2name[nRows] = name;
    m_name2Row[name] = nRows;
    m_nodes.insert(name, pItem);

    endInsertRows();
    pItem->params->setNodeIdx(createIndex(nRows, 0));
#endif
}

bool GraphModel::removeNode(const QString& name)
{
    if (m_name2uuid.find(name) != m_name2uuid.end() && m_nodes.find(m_name2uuid[name]) != m_nodes.end()) {
        return _removeNodeImpl(name, true);
    }
    return false;
}

void GraphModel::setView(const QModelIndex& idx, bool bOn)
{
    _setViewImpl(idx, bOn, true);
}

void GraphModel::setMute(const QModelIndex& idx, bool bOn)
{
    _setMuteImpl(idx, bOn, true);
}

QString GraphModel::updateNodeName(const QModelIndex& idx, QString newName)
{
    auto spCoreGraph = m_impl->m_wpCoreGraph;
    ZASSERT_EXIT(spCoreGraph, false);

    std::string oldName = idx.data(QtRole::ROLE_NODE_NAME).toString().toStdString();
    newName = QString::fromStdString(spCoreGraph->updateNodeName(oldName, newName.toStdString()));
    return newName;
}

void GraphModel::updateSocketValue(const QModelIndex& nodeidx, const QString socketName, const QVariant newValue)
{
    m_impl->m_wpCoreGraph->hasNode("");
    if (ParamsModel* paramModel = params(nodeidx))
    {
        QModelIndex& socketIdx = paramModel->paramIdx(socketName, true);
        paramModel->setData(socketIdx, newValue, QtRole::ROLE_PARAM_VALUE);
    }
}

QHash<int, QByteArray> GraphModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[QtRole::ROLE_CLASS_NAME] = "classname";
    roles[QtRole::ROLE_NODE_NAME] = "name";
    //roles[QtRole::ROLE_NODE_UISTYLE] = "uistyle";
    roles[QtRole::ROLE_PARAMS] = "params";
    roles[QtRole::ROLE_LINKS] = "linkModel";
    roles[QtRole::ROLE_OBJPOS] = "pos";
    roles[QtRole::ROLE_SUBGRAPH] = "subgraph";
    return roles;
}

void GraphModel::_clear()
{
    while (rowCount() > 0) {
        //only delete ui model element itself, and then unregister from core.
        removeRows(0, 1);
    }
}

bool GraphModel::removeRows(int row, int count, const QModelIndex& parent)
{
    //this is a private impl method, called by callback function.
    beginRemoveRows(parent, row, row);

    QString id = m_row2uuid[row];
    NodeItem* pItem = m_nodes[id];
    const QString& name = pItem->getName();

    for (int r = row + 1; r < rowCount(); r++)
    {
        const QString& id_ = m_row2uuid[r];
        m_row2uuid[r - 1] = id_;
        m_uuid2Row[id_] = r - 1;
    }

    m_row2uuid.remove(rowCount() - 1);
    m_uuid2Row.remove(id);
    m_nodes.remove(id);
    m_name2uuid.remove(name);

    if (m_subgNodes.find(id) != m_subgNodes.end())
        m_subgNodes.remove(id);

    delete pItem;

    endRemoveRows();

    emit nodeRemoved(name);
    return true;
}

void GraphModel::syncToAssetsInstance(const QString& assetsName, zeno::ParamsUpdateInfo info, const zeno::CustomUI& customui)
{
    QModelIndexList results = match(QModelIndex(), QtRole::ROLE_CLASS_NAME, assetsName);
    for (QModelIndex res : results) {
        zeno::NodeType type = (zeno::NodeType)res.data(QtRole::ROLE_NODETYPE).toInt();
        if (type == zeno::Node_AssetInstance || type == zeno::Node_AssetReference) {
            ParamsModel* paramsM = QVariantPtr<ParamsModel>::asPtr(res.data(QtRole::ROLE_PARAMS));
            ZASSERT_EXIT(paramsM);
            paramsM->resetCustomUi(customui);
            paramsM->batchModifyParams(info);
        }
    }

    const QStringList& path = currentPath();
    if (!path.isEmpty() && path[0] != "main") {
        return;
    }

    syncToAssetsInstance(assetsName);

    for (QString subgnode : m_subgNodes) {
        ZASSERT_EXIT(m_name2uuid.find(subgnode) != m_name2uuid.end());
        QString uuid = m_name2uuid[subgnode];
        ZASSERT_EXIT(m_nodes.find(uuid) != m_nodes.end());
        GraphModel* pSubgM = m_nodes[uuid]->optSubgraph.value();
        ZASSERT_EXIT(pSubgM);
        pSubgM->syncToAssetsInstance(assetsName, info, customui);
    }
}

void GraphModel::syncToAssetsInstance(const QString& assetsName)
{
    for (const QString & name : m_subgNodes)
    {
        ZASSERT_EXIT(m_name2uuid.find(name) != m_name2uuid.end());
        QString uuid = m_name2uuid[name];
        ZASSERT_EXIT(m_nodes.find(uuid) != m_nodes.end());
        GraphModel* pSubgM = m_nodes[uuid]->optSubgraph.value();
        ZASSERT_EXIT(pSubgM);
        if (assetsName == m_nodes[uuid]->cls)
        {
            //TO DO: compare diff
            if (!pSubgM->isLocked())
                continue;
            while (pSubgM->rowCount() > 0)
            {
                const QString& nodeName = pSubgM->index(0, 0).data(QtRole::ROLE_NODE_NAME).toString();
                pSubgM->removeNode(nodeName);
            }
            auto spNode = m_nodes[uuid]->m_wpNode;
            auto spSubnetNode = dynamic_cast<zeno::SubnetNode*>(spNode->coreNode());
            if (spSubnetNode) {
                auto& assetsMgr = zeno::getSession().assets;
                assetsMgr->updateAssetInstance(assetsName.toStdString(), spSubnetNode);
                //pSubgM->updateAssetInstance(spSubnetNode->subgraph.get());
                {
                    auto spGraph = spSubnetNode->subgraph.get();
                    pSubgM->m_impl->m_wpCoreGraph = spGraph;
                    pSubgM->registerCoreNotify();
                    for (auto& [name, node] : spGraph->getNodes()) {
                        pSubgM->_appendNode(node);
                    }
                    pSubgM->_initLink();
                }
                spNode->mark_dirty(true);
            }
        }
        else
        {
            pSubgM->syncToAssetsInstance(assetsName);
        }
    }
}

void GraphModel::updateParamName(QModelIndex nodeIdx, int row, QString newName)
{
    NodeItem* item = m_nodes[m_row2uuid[nodeIdx.row()]];
    QModelIndex paramIdx = item->params->index(row, 0);
    item->params->setData(paramIdx, newName, QtRole::ROLE_PARAM_NAME);
}

void GraphModel::removeParam(QModelIndex nodeIdx, int row)
{
    NodeItem* item = m_nodes[m_row2uuid[nodeIdx.row()]];
    item->params->removeRow(row);
}

ParamsModel* GraphModel::params(QModelIndex nodeIdx)
{
    NodeItem* item = m_nodes[m_row2uuid[nodeIdx.row()]];
    return item->params;
}

GraphModel* GraphModel::subgraph(QModelIndex nodeIdx) {
    NodeItem* item = m_nodes[m_row2uuid[nodeIdx.row()]];
    if (item->optSubgraph.has_value()) {
        return item->optSubgraph.value();
    }
    return nullptr;
}

GraphsTreeModel* GraphModel::treeModel() const {
    return m_pTree;
}

void GraphModel::setLocked(bool bLocked)
{
    m_bLocked = bLocked;
    emit lockStatusChanged();
}

bool GraphModel::isLocked() const
{
    return m_bLocked;
}

void GraphModel::importNodes(const zeno::NodesData& nodes, const zeno::LinksData& links, const QPointF& pos)
{
    if (nodes.empty())
        return;

    std::map<std::string, std::string> old2new;
    QPointF offset = pos - QPointF(nodes.begin()->second.uipos.first, nodes.begin()->second.uipos.second);
    unRegisterCoreNotify();
    for (auto [name, node] : nodes) {
        bool bAsset = node.asset.has_value();
        auto spNode = m_impl->m_wpCoreGraph->createNode(node.cls, "", bAsset, { offset.x(), offset.y() });
        node.name = spNode->m_pImpl->get_name();
        spNode->m_pImpl->init(node);
        auto pos = spNode->m_pImpl->get_pos();
        spNode->m_pImpl->set_pos({ pos.first + offset.x(), pos.second + offset.y()});
        old2new[name] = node.name;
        _appendNode(spNode);
    }
    registerCoreNotify();
    //import edges
    for (auto link : links) {
        if (old2new.find(link.outNode) == old2new.end() || old2new.find(link.inNode) == old2new.end())
            continue;
        link.inNode = old2new[link.inNode];
        link.outNode = old2new[link.outNode];
        addLink(link);
    }
}

GraphModel* GraphModel::getTopLevelGraph(const QStringList& currentPath)
{
    return zenoApp->graphsManager()->getGraph({ currentPath[0] });
}

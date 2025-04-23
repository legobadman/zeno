﻿#include "nodecatemodel.h"
#include "graphsmanager.h"
#include "zenoapplication.h"
#include "zassert.h"
#include "nodeeditor/gv/fuzzy_search.h"
#include "model/GraphModel.h"
#include "model/pluginsmodel.h"

//#define DISABLE_CATE

NodeCateModel::NodeCateModel(QObject* parent) : _base(parent) {
    reload();

    PluginsModel* pluginsM = zenoApp->graphsManager()->pluginModel();
    connect(pluginsM, &PluginsModel::rowsInserted, this, &NodeCateModel::reload);
    connect(pluginsM, &PluginsModel::rowsRemoved, this, &NodeCateModel::reload);
}

void NodeCateModel::reload() {
    m_items.clear();
    m_cache_cates.clear();
    m_nodeToCate.clear();
    m_condidates.clear();

    const NodeCates& cates = zenoApp->graphsManager()->getCates();
#ifdef DISABLE_CATE
    m_cache_cates.resize(1);
#else
    m_cache_cates.resize(cates.size());
#endif
    int i = 0;
    for (QString cate : cates.keys()) {
#ifdef DISABLE_CATE
        if (cate != "reflect" && cate != "prim" && cate != "subgraph" && cate != "layout")
            continue;
#endif

#ifdef DISABLE_CATE
        if (cate == "reflect")
#endif
            QVector<zeno::NodeInfo> nodes = cates[cate];
        {
            m_cache_cates[i].name = cate;
            std::transform(nodes.begin(), nodes.end(), std::back_inserter(m_cache_cates[i].nodes),
                [](const zeno::NodeInfo& v) { return QString::fromStdString(v.name); });
            i++;
        }
        for (const auto& node : nodes) {
            QString nodecls = QString::fromStdString(node.name);
            if (nodecls == "SubInput" || nodecls == "SubOutput")
                continue;
            m_nodeToCate[nodecls] = cate;
            m_condidates.push_back(nodecls);
        }
    }

    m_items = m_cache_cates;
    //TODO: sync with graphsmanager when assets insert/remove.
}

void NodeCateModel::initNewNodeCase() {
    MenuOrItem item;
    item.category = "control";
    item.newcase = Case_Foreach_Count;
    item.name = "ForEach-Count";
    m_items.push_back(item);

    item.category = "control";
    item.newcase = Case_Foreach_PrimAttr;
    item.name = "ForEach-Prim-Attribute";
    m_items.push_back(item);
}

int NodeCateModel::rowCount(const QModelIndex& parent) const {
    return m_items.size();
}

QVariant NodeCateModel::data(const QModelIndex& index, int role) const {
    switch (role)
    {
    case QmlNodeCateRole::Name:  return m_items[index.row()].name;
    case QmlNodeCateRole::CateNodes: return m_items[index.row()].nodes;
    case QmlNodeCateRole::IsCategory:return m_items[index.row()].iscate;
    case QmlNodeCateRole::Keywords:  return m_items[index.row()].matchIndices;
    case QmlNodeCateRole::Category:  return m_items[index.row()].category;
    case QmlNodeCateRole::LastItem:  return m_items[index.row()].islastitem;
    }
}

bool NodeCateModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    return _base::setData(index, value, role);
}

QHash<int, QByteArray> NodeCateModel::roleNames() const
{
    QHash<int, QByteArray> values;
    values[QmlNodeCateRole::Name] = "name";
    values[QmlNodeCateRole::CateNodes] = "nodelist";
    values[QmlNodeCateRole::IsCategory] = "iscate";
    values[QmlNodeCateRole::Keywords] = "keywords";
    values[QmlNodeCateRole::Category] = "category";
    values[QmlNodeCateRole::LastItem] = "lastitem";
    return values;
}

bool NodeCateModel::removeRows(int row, int count, const QModelIndex& parent) {
    beginRemoveRows(parent, row, row);
    m_items.removeAt(row);
    endRemoveRows();
    return true;
}

void NodeCateModel::clear() {
    beginResetModel();
    m_items.clear();
    endResetModel();
}

bool NodeCateModel::iscatepage() const {
    return m_search.isEmpty();
}

bool NodeCateModel::execute(GraphModel* pGraphM, const QString& name, const QPoint& pt) {
    if (m_nodeToCate.find(name) == m_nodeToCate.end()) {
        zeno::log_error("cannot find {}", name.toStdString());
        return false;
    }
    const QString& cate = m_nodeToCate[name];

    NodeCates m_cates = zenoApp->graphsManager()->getCates();
    const QVector<zeno::NodeInfo>& nodes = m_cates[cate];
    for (zeno::NodeInfo node_info : nodes) {
        if (node_info.name == name.toStdString() &&
            node_info.status == zeno::ZModule_UnLoaded) {
            return false;
        }
    }

    if (cate != "control") {
        pGraphM->createNode(name, cate, pt);
    }
    else {
        if (name == "Foreach-Count") {
            zeno::NodeData foreachbegin = pGraphM->createNode("ForEachBegin", "reflect", pt); //TODO: cate以后优化一下
            zeno::NodeData foreachend = pGraphM->createNode("ForEachEnd", "reflect", QPointF(pt.x(), pt.y() + 400));

            const QString& beginNode = QString::fromStdString(foreachbegin.name);
            const QString& endNode = QString::fromStdString(foreachend.name);

            pGraphM->addLink(beginNode, "Output Object", endNode, "Iterate Object");

            //TODO: 以后再加上分组框
            //QStringList uuids;
            //uuids.append(pGraphM->indexFromName(beginNode).data(QtRole::ROLE_NODE_UUID_PATH).toString());
            //uuids.append(pGraphM->indexFromName(endNode).data(QtRole::ROLE_NODE_UUID_PATH).toString());
            //emit pGraphM->nodesAboutToBeGroup(uuids);
        }
        else if (name == "Foreach-Geometry-attr") {

        }
        else if (name == "Foreach-StopCond") {

        }
    }
    return true;
}

void NodeCateModel::search(const QString& name) {
    zeno::scope_exit sp([&]() { m_search = name; });

    clear();
    if (name.isEmpty()) {
        //搜索项是空的，于是把分类菜单加回来
        beginInsertRows(QModelIndex(), 0, m_cache_cates.size() - 1);
        m_items = m_cache_cates;
        endInsertRows();
        return;
    }
    else {
        const QList<FuzzyMatchKey>& searchResult = fuzzy_search(name, m_condidates);
        if (searchResult.isEmpty()) {
            return;
        }
        const int N = searchResult.size();
        beginInsertRows(QModelIndex(), 0, N - 1);
        int deprecatedIndex = N;
        for (int i = 0; i < N; i++) {
            auto& [name, matchIndices] = searchResult[i];
            MenuOrItem newitem;
            newitem.name = name;
            const QString& category = m_nodeToCate[name];  //TODO：资产名和节点同名的情况
            newitem.category = category;
            newitem.iscate = false;
            for (auto idx : matchIndices) {
                newitem.matchIndices.append(idx);
            }
            if (i == N - 1) {
                newitem.islastitem = true;
            }
            m_items.push_back(newitem);
        }
        endInsertRows();
    }
}

#include "nodecatemodel.h"
#include "graphsmanager.h"
#include "zenoapplication.h"
#include "zassert.h"
#include "nodeeditor/gv/fuzzy_search.h"


NodeCateModel::NodeCateModel(QObject* parent) : _base(parent) {
    zeno::NodeCates cates = zenoApp->graphsManager()->getCates();
    m_cache_cates.resize(cates.size());
    int i = 0;
    for (auto& [cate, nodes] : cates) {
        m_cache_cates[i].name = QString::fromStdString(cate);
        std::transform(nodes.begin(), nodes.end(), std::back_inserter(m_cache_cates[i].nodes), [](const std::string& v) { return QString::fromStdString(v); });
        i++;

        for (const auto& node : nodes) {
            QString nodecls = QString::fromStdString(node);
            if (nodecls == "SubInput" || nodecls == "SubOutput")
                continue;
            m_nodeToCate[nodecls] = QString::fromStdString(cate);
            m_condidates.push_back(nodecls);
        }
    }
    m_items = m_cache_cates;
    //TODO: sync with graphsmanager when assets insert/remove.
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
        beginInsertRows(QModelIndex(), 0, searchResult.size() - 1);
        int deprecatedIndex = searchResult.size();
        for (int i = 0; i < searchResult.size(); i++) {
            auto& [name, matchIndices] = searchResult[i];
            MenuOrItem newitem;
            newitem.name = name;
            const QString& category = m_nodeToCate[name];  //TODO：资产名和节点同名的情况
            newitem.category = category;
            newitem.iscate = false;
            for (auto idx : matchIndices) {
                newitem.matchIndices.append(idx);
            }
            m_items.push_back(newitem);
        }
        endInsertRows();
    }
}

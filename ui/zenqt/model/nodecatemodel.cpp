#include "nodecatemodel.h"
#include "graphsmanager.h"
#include "zenoapplication.h"
#include "zassert.h"
#include "nodeeditor/gv/fuzzy_search.h"


NodeCateModel::NodeCateModel(bool bShowResult, QObject* parent)
    : _base(parent)
    , m_bShowResult(bShowResult)
{
    zeno::NodeCates cates = zenoApp->graphsManager()->getCates();
    m_cache_cates.resize(cates.size());
    int i = 0;
    for (auto& [cate, nodes] : cates) {
        m_cache_cates[i].cate = QString::fromStdString(cate);
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
    if (!m_bShowResult) {
        m_cates = m_cache_cates;
    }
    //TODO: sync with graphsmanager when assets insert/remove.
}

int NodeCateModel::rowCount(const QModelIndex& parent) const {
    return m_cates.size();
}

QVariant NodeCateModel::data(const QModelIndex& index, int role) const {
    switch (role)
    {
    case QmlNodeCateRole::Name:  return m_cates[index.row()].cate;
    case QmlNodeCateRole::CateNodes: return m_cates[index.row()].nodes;
    case QmlNodeCateRole::IsCategory:return m_cates[index.row()].iscate;
    case QmlNodeCateRole::Keywords:  return m_cates[index.row()].matchIndices;
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
    return values;
}

bool NodeCateModel::removeRows(int row, int count, const QModelIndex& parent) {
    beginRemoveRows(parent, row, row);
    m_cates.removeAt(row);
    endRemoveRows();
    return true;
}

void NodeCateModel::clear() {
    beginResetModel();
    m_cates.clear();
    endResetModel();
}

bool NodeCateModel::iscatepage() const {
    return m_search.isEmpty();
}

void NodeCateModel::search(const QString& name) {
    zeno::scope_exit sp([&]() { m_search = name; });

    if (!m_bShowResult)
        return;

    clear();
    if (name.isEmpty()) {
        return;
    }
    else {
        const QList<FuzzyMatchKey>& searchResult = fuzzy_search(name, m_condidates);
        if (!searchResult.isEmpty())
        {
            beginInsertRows(QModelIndex(), 0, searchResult.size() - 1);
            int deprecatedIndex = searchResult.size();
            for (int i = 0; i < searchResult.size(); i++) {
                auto& [name, matchIndices] = searchResult[i];
                _CateItem newitem;
                newitem.cate = name;
                newitem.iscate = false;
                for (auto idx : matchIndices) {
                    newitem.matchIndices.append(idx);
                }
                m_cates.push_back(newitem);
            }
            endInsertRows();
        }
    }
}

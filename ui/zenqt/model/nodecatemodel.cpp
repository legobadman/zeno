#include "nodecatemodel.h"
#include "graphsmanager.h"
#include "zenoapplication.h"
#include "zassert.h"


NodeCateModel::NodeCateModel(QObject* parent) : _base(parent) {
    zeno::NodeCates cates = zenoApp->graphsManager()->getCates();
    m_cates.resize(cates.size());
    int i = 0;
    for (auto& [cate, lst] : cates) {
        m_cates[i].cate = QString::fromStdString(cate);
        std::transform(lst.begin(), lst.end(), std::back_inserter(m_cates[i].nodes), [](const std::string& v) { return QString::fromStdString(v); });
        i++;
    }
    //TODO: sync with graphsmanager when assets insert/remove.
}

int NodeCateModel::rowCount(const QModelIndex& parent) const {
    return m_cates.size();
}

QVariant NodeCateModel::data(const QModelIndex& index, int role) const {
    switch (role)
    {
    case QmlNodeCateRole::Category:  return m_cates[index.row()].cate;
    case QmlNodeCateRole::CateNodes: return m_cates[index.row()].nodes;
    case QmlNodeCateRole::IsCategory:return m_cates[index.row()].iscate;
    case QmlNodeCateRole::Keywords:  return m_search;
    }
}

bool NodeCateModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    return _base::setData(index, value, role);
}

QHash<int, QByteArray> NodeCateModel::roleNames() const
{
    QHash<int, QByteArray> values;
    values[QmlNodeCateRole::Category] = "category";
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
    //removeRows(0, 1);
    if (m_search.isEmpty()) {
        //清空原有所有的分类菜单项
        //removeRows(0, rowCount());
        clear();
        //添加搜索项
        beginInsertRows(QModelIndex(), 0, 1);
        _CateItem newitem;
        newitem.cate = name;
        newitem.iscate = false;
        //newitem.nodes = QStringList({ "abc", "dec" });
        m_cates.push_back(newitem);
        newitem.cate = "f2";
        m_cates.push_back(newitem);
        endInsertRows();
    }
    else if (name.length() < m_search.length()) {
        //关键字删减，搜索范围要扩大，还是把之前的全清理掉
        clear();
        if (name.isEmpty()) {
            //把分类菜单加回来
            zeno::NodeCates cates = zenoApp->graphsManager()->getCates();
            beginInsertRows(QModelIndex(), 0, cates.size() - 1);
            m_cates.resize(cates.size());
            int i = 0;
            for (auto& [cate, lst] : cates) {
                m_cates[i].cate = QString::fromStdString(cate);
                std::transform(lst.begin(), lst.end(), std::back_inserter(m_cates[i].nodes), [](const std::string& v) { return QString::fromStdString(v); });
                i++;
            }
            endInsertRows();
        }
    }
    else {
        ZASSERT_EXIT(name.length() > m_search.length());
    }

    //beginInsertRows(QModelIndex(), m_cates.size() - 1, m_cates.size() - 1);
    //_CateItem newitem;
    //newitem.cate = "fuckyou";
    //newitem.nodes = QStringList({ "abc", "dec" });
    //m_cates.push_back(newitem);
    //endInsertRows();
    m_search = name;
}

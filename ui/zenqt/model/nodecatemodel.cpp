#include "nodecatemodel.h"
#include "graphsmanager.h"
#include "zenoapplication.h"


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
    if (role == Qt::DisplayRole) {
        return m_cates[index.row()].cate;
    }
    else {
        return m_cates[index.row()].nodes;
    }
}

bool NodeCateModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    return _base::setData(index, value, role);
}

QHash<int, QByteArray> NodeCateModel::roleNames() const
{
    QHash<int, QByteArray> values;
    values[Qt::DisplayRole] = "category";
    values[Qt::DecorationRole] = "nodelist";
    return values;
}

bool NodeCateModel::removeRows(int row, int count, const QModelIndex& parent) {
    beginRemoveRows(parent, row, row);
    m_cates.removeAt(row);
    endRemoveRows();
    return true;
}

void NodeCateModel::search(const QString& name) {
    //removeRows(0, 1);

    //beginInsertRows(QModelIndex(), m_cates.size() - 1, m_cates.size() - 1);
    //_CateItem newitem;
    //newitem.cate = "fuckyou";
    //newitem.nodes = QStringList({ "abc", "dec" });
    //m_cates.push_back(newitem);
    //endInsertRows();
}

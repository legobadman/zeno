#include "customuimodel.h"


CustomUIModel::CustomUIModel(ParamsModel* params, QObject* parent)
    : QObject(parent)
    , m_params(params)
    , m_tabModel(nullptr)
    , m_outputModel(nullptr)
{
    const zeno::CustomUI& customUI = m_params->customUI();
    m_tabModel = new ParamTabModel(customUI.inputPrims, this);
    m_outputModel = new ParamOutputModel(customUI.outputPrims, this);
}

ParamTabModel* CustomUIModel::tabModel() const
{
    return m_tabModel;
}

ParamOutputModel* CustomUIModel::outputModel() const
{
    return m_outputModel;
}

ParamsModel* CustomUIModel::coreModel() const {
    return m_params;
}


//////////////////////////////////////////////////////////
ParamTabModel::ParamTabModel(zeno::CustomUIParams tabs, CustomUIModel* pModel)
    : QAbstractListModel(pModel)
    , m_customuiM(pModel)
{
    for (const zeno::ParamTab& tab : tabs) {
        _TabItem item;
        item.name = QString::fromStdString(tab.name);
        item.groupM = new ParamGroupModel(tab, this);
        m_items.push_back(std::move(item));
    }
}

int ParamTabModel::rowCount(const QModelIndex& parent) const {
    return m_items.size();
}

QVariant ParamTabModel::data(const QModelIndex& index, int role) const {
    if (role == Qt::DisplayRole) {
        return m_items[index.row()].name;
    }
    else if (role == QmlCUIRole::GroupModel) {
        return QVariant::fromValue(m_items[index.row()].groupM);
    }
    else {
        return QVariant();
    }
}

bool ParamTabModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    return false;
}

QHash<int, QByteArray> ParamTabModel::roleNames() const {
    QHash<int, QByteArray> values;
    values[Qt::DisplayRole] = "name";
    values[QmlCUIRole::GroupModel] = "groups";
    return values;
}


//////////////////////////////////////////////////
ParamGroupModel::ParamGroupModel(zeno::ParamTab tab, ParamTabModel* pModel)
    : QAbstractListModel(pModel)
    , m_tabModel(pModel)
{
    for (const zeno::ParamGroup& group : tab.groups) {
        _GroupItem item;
        item.name = QString::fromStdString(group.name);
        item.paramM = new ParamPlainModel(group, this);
        m_items.push_back(std::move(item));
    }
}

int ParamGroupModel::rowCount(const QModelIndex& parent) const {
    return m_items.size();
}

QVariant ParamGroupModel::data(const QModelIndex& index, int role) const {
    if (role == Qt::DisplayRole) {
        return m_items[index.row()].name;
    }
    else if (role == QmlCUIRole::PrimModel) {
        return QVariant::fromValue(m_items[index.row()].paramM);
    }
    return QVariant();
}

bool ParamGroupModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    return false;
}

QHash<int, QByteArray> ParamGroupModel::roleNames() const {
    QHash<int, QByteArray> values;
    values[Qt::DisplayRole] = "name";
    values[QmlCUIRole::PrimModel] = "params";
    return values;
}


////////////////////////////////////////////////////////
ParamPlainModel::ParamPlainModel(zeno::ParamGroup group, ParamGroupModel* pModel)
    : QAbstractListModel(pModel)
    , m_groupModel(pModel)
{
    ParamsModel* paramM = m_groupModel->tabModel()->uimodel()->coreModel();
    for (const zeno::ParamPrimitive& param : group.params) {
        int r = paramM->indexFromName(QString::fromStdString(param.name), true);
        QPersistentModelIndex idx = paramM->index(r);
        m_items.push_back(idx);
    }
}

int ParamPlainModel::rowCount(const QModelIndex& parent) const {
    return m_items.size();
}

QVariant ParamPlainModel::data(const QModelIndex& index, int role) const {
    return m_items[index.row()].data(role);
}

bool ParamPlainModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    return false;
}

QHash<int, QByteArray> ParamPlainModel::roleNames() const {
    //copy from ParamsModel::roleNames()
    QHash<int, QByteArray> roles;
    roles[QtRole::ROLE_NODE_NAME] = "nodename";
    roles[QtRole::ROLE_PARAM_NAME] = "name";
    roles[QtRole::ROLE_PARAM_TYPE] = "type";
    roles[QtRole::ROLE_PARAM_CONTROL] = "control";
    roles[QtRole::ROLE_ISINPUT] = "input";
    roles[QtRole::ROLE_PARAM_GROUP] = "group";
    roles[QtRole::ROLE_PARAM_SOCKET_VISIBLE] = "socket_visible";
    roles[QtRole::ROLE_PARAM_CONTROL_PROPS] = "control_properties";
    roles[QtRole::ROLE_PARAM_SOCKET_CLR] = "socket_color";
    return roles;
}


///////////////////////////////////////////////////////////////
ParamOutputModel::ParamOutputModel(zeno::PrimitiveParams params, CustomUIModel* pModel)
    : QAbstractListModel(pModel)
{
    ParamsModel* paramM = pModel->coreModel();
    for (const zeno::ParamPrimitive& param : params) {
        int r = paramM->indexFromName(QString::fromStdString(param.name), true);
        QPersistentModelIndex idx = paramM->index(r);
        m_items.push_back(idx);
    }
}

int ParamOutputModel::rowCount(const QModelIndex& parent) const {
    return m_items.size();
}

QVariant ParamOutputModel::data(const QModelIndex& index, int role) const {
    return m_items[index.row()].data(role);
}

bool ParamOutputModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    return false;
}

QHash<int, QByteArray> ParamOutputModel::roleNames() const {
    //copy from ParamsModel::roleNames()
    QHash<int, QByteArray> roles;
    roles[QtRole::ROLE_NODE_NAME] = "nodename";
    roles[QtRole::ROLE_PARAM_NAME] = "name";
    roles[QtRole::ROLE_PARAM_TYPE] = "type";
    roles[QtRole::ROLE_PARAM_CONTROL] = "control";
    roles[QtRole::ROLE_ISINPUT] = "input";
    roles[QtRole::ROLE_PARAM_GROUP] = "group";
    roles[QtRole::ROLE_PARAM_SOCKET_VISIBLE] = "socket_visible";
    roles[QtRole::ROLE_PARAM_CONTROL_PROPS] = "control_properties";
    roles[QtRole::ROLE_PARAM_SOCKET_CLR] = "socket_color";
    return roles;
}
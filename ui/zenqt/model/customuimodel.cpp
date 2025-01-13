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

void CustomUIModel::initCustomuiConnections(QStandardItemModel* customuiStandarditemModel)
{
    connect(customuiStandarditemModel, &QStandardItemModel::rowsInserted, [customuiStandarditemModel, this](const QModelIndex& parent, int first, int last) {
        QStandardItem* parentItem = customuiStandarditemModel->itemFromIndex(parent);
        QStandardItem* newItem = parentItem->child(first);
        int elemtype = newItem->data(ROLE_ELEMENT_TYPE).toInt();
        ParamTabModel* tabmodel = tabModel();
        if (elemtype == VPARAM_TAB) {
            tabmodel->insertRow(first, newItem->data(QtRole::ROLE_PARAM_NAME).toString());
        } 
        else if (elemtype == VPARAM_GROUP) {
            ParamGroupModel* group = tabmodel->index(parent.row()).data(QmlCUIRole::GroupModel).value<ParamGroupModel*>();
            group->insertRow(first, newItem->data(QtRole::ROLE_PARAM_NAME).toString());
        }
        else if (elemtype == VPARAM_PARAM) {
            QModelIndex& tabindx = parent.parent();
            if (ParamGroupModel* groupmodel = tabmodel->index(tabindx.row()).data(QmlCUIRole::GroupModel).value<ParamGroupModel*>()) {
                ParamPlainModel* paramsmodel = groupmodel->index(parent.row()).data(QmlCUIRole::PrimModel).value<ParamPlainModel*>();
                paramsmodel->insertRow(first, newItem->data(QtRole::ROLE_PARAM_NAME).toString());
            }
        }
    });
    connect(customuiStandarditemModel, &QStandardItemModel::rowsAboutToBeRemoved, [customuiStandarditemModel, this](const QModelIndex& parent, int first, int last) {
        QStandardItem* parentItem = customuiStandarditemModel->itemFromIndex(parent);
        QStandardItem* removeItem = parentItem->child(first);
        int elemtype = removeItem->data(ROLE_ELEMENT_TYPE).toInt();
        if (elemtype == VPARAM_TAB) {
            ParamTabModel* tabmodel = tabModel();
            tabmodel->removeRow(first);
        }
        });
    connect(customuiStandarditemModel, &QStandardItemModel::dataChanged, [customuiStandarditemModel, this](const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles) {
        if (roles.size() == 1 && roles[0] == QtRole::ROLE_PARAM_NAME) {
            if (topLeft.data(ROLE_ELEMENT_TYPE).toInt() == VPARAM_TAB) {
                ParamTabModel* tabmodel = tabModel();
                tabmodel->setData(topLeft, topLeft.data(QtRole::ROLE_PARAM_NAME));
            }
        }
    });
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
    switch (role) {
    case QtRole::ROLE_PARAM_NAME:
        m_items[index.row()].name = value.toString();
        return true;
    }
    return false;
}

QHash<int, QByteArray> ParamTabModel::roleNames() const {
    QHash<int, QByteArray> values;
    values[Qt::DisplayRole] = "name";
    values[QmlCUIRole::GroupModel] = "groups";
    return values;
}


bool ParamTabModel::insertRow(int row, QString& name) {
    if (row < 0 || row > m_items.size()) {
        return false;
    }

    beginInsertRows(QModelIndex(), row, row);
    _TabItem item;
    item.name = name;
    item.groupM = new ParamGroupModel(zeno::ParamTab{name.toStdString()}, this);
    m_items.insert(row, std::move(item));
    endInsertRows();
    return true;
}

bool ParamTabModel::removeRow(int row)
{
    if (row < 0 || row >= m_items.size()) {
        return false;
    }

    beginRemoveRows(QModelIndex(), row, row);
    m_items.erase(m_items.begin() + row);
    endRemoveRows();
    return true;
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


bool ParamGroupModel::insertRow(int row, QString& name)
{
    return false;
}

bool ParamGroupModel::removeRow(int row)
{
    return false;
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
    roles[QtRole::ROLE_PARAM_NAME] = "paramname";
    roles[QtRole::ROLE_PARAM_TYPE] = "type";
    roles[QtRole::ROLE_PARAM_CONTROL] = "control";
    roles[QtRole::ROLE_PARAM_QML_VALUE] = "value";
    roles[QtRole::ROLE_ISINPUT] = "input";
    roles[QtRole::ROLE_PARAM_GROUP] = "group";
    roles[QtRole::ROLE_PARAM_SOCKET_VISIBLE] = "socket_visible";
    roles[QtRole::ROLE_PARAM_CONTROL_PROPS] = "control_properties";
    roles[QtRole::ROLE_PARAM_SOCKET_CLR] = "socket_color";
    return roles;
}


bool ParamPlainModel::insertRow(int row, QString& name)
{
    return false;
}

bool ParamPlainModel::removeRow(int row)
{
    return false;
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
    roles[QtRole::ROLE_PARAM_NAME] = "paramname";
    roles[QtRole::ROLE_PARAM_TYPE] = "type";
    roles[QtRole::ROLE_PARAM_CONTROL] = "control";
    roles[QtRole::ROLE_ISINPUT] = "input";
    roles[QtRole::ROLE_PARAM_GROUP] = "group";
    roles[QtRole::ROLE_PARAM_SOCKET_VISIBLE] = "socket_visible";
    roles[QtRole::ROLE_PARAM_CONTROL_PROPS] = "control_properties";
    roles[QtRole::ROLE_PARAM_SOCKET_CLR] = "socket_color";
    return roles;
}
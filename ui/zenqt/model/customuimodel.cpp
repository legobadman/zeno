#include "customuimodel.h"
#include <zeno/utils/helper.h>
#include "zeno_types/reflect/reflection.generated.hpp"

static CUSTOMUI_CTRL_ITEM_INFO customui_controlList[] = {
    {"Tab",                 zeno::NullControl,  Param_Null,   "qrc:/icons/parameter_control_tab.svg"},
    {"Group",               zeno::NullControl,  Param_Null,   "qrc:/icons/parameter_control_group.svg"},
    {"object",              zeno::NullControl,  Param_Null,   ""},
    {"Integer",             zeno::Lineedit,     zeno::types::gParamType_Int,    "qrc:/icons/parameter_control_integer.svg"},
    {"Float",               zeno::Lineedit,     zeno::types::gParamType_Float,  "qrc:/icons/parameter_control_float.svg"},
    {"String",              zeno::Lineedit,     zeno::types::gParamType_String, "qrc:/icons/parameter_control_string.svg"},
    {"Boolean",             zeno::Checkbox,     zeno::types::gParamType_Bool,   "qrc:/icons/parameter_control_boolean.svg"},
    {"Multiline String",    zeno::Multiline,    zeno::types::gParamType_String, "qrc:/icons/parameter_control_string.svg"},
    {"read path",           zeno::ReadPathEdit, zeno::types::gParamType_String, "qrc:/icons/parameter_control_fold.svg"},
    {"write path",          zeno::WritePathEdit,zeno::types::gParamType_String, "qrc:/icons/parameter_control_fold.svg"},
    {"directory",       zeno::DirectoryPathEdit,zeno::types::gParamType_String, "qrc:/icons/parameter_control_fold.svg"},
    {"Enum",                zeno::Combobox,     zeno::types::gParamType_String, "qrc:/icons/parameter_control_enum.svg"},
    {"Float Vector 4",      zeno::Vec4edit,     zeno::types::gParamType_Vec4f,  "qrc:/icons/parameter_control_floatVector4.svg"},
    {"Float Vector 3",      zeno::Vec3edit,     zeno::types::gParamType_Vec3f,  "qrc:/icons/parameter_control_floatVector3.svg"},
    {"Float Vector 2",      zeno::Vec2edit,     zeno::types::gParamType_Vec2f,  "qrc:/icons/parameter_control_floatVector2.svg"},
    {"Integer Vector 4",    zeno::Vec4edit,     zeno::types::gParamType_Vec4i,  "qrc:/icons/parameter_control_integerVector4.svg"},
    {"Integer Vector 3",    zeno::Vec3edit,     zeno::types::gParamType_Vec3i,  "qrc:/icons/parameter_control_integerVector3.svg"},
    {"Integer Vector 2",    zeno::Vec2edit,     zeno::types::gParamType_Vec2i,  "qrc:/icons/parameter_control_integerVector2.svg"},
    {"Color",               zeno::Heatmap,      zeno::types::gParamType_Heatmap,"qrc:/icons/parameter_control_color.svg"},
    {"Color Vec3f",         zeno::ColorVec,     zeno::types::gParamType_Vec3f,  "qrc:/icons/parameter_control_color.svg"},
    {"Curve",               zeno::CurveEditor,  zeno::types::gParamType_Curve,  "qrc:/icons/parameter_control_curve.svg"},
    {"SpinBox",             zeno::SpinBox,      zeno::types::gParamType_Int,    "qrc:/icons/parameter_control_spinbox.svg"},
    {"DoubleSpinBox",       zeno::DoubleSpinBox,zeno::types::gParamType_Float,  "qrc:/icons/parameter_control_spinbox.svg"},
    {"Slider",              zeno::Slider,       zeno::types::gParamType_Int,    "qrc:/icons/parameter_control_slider.svg"},
    {"SpinBoxSlider",       zeno::SpinBoxSlider,zeno::types::gParamType_Int,    "qrc:/icons/parameter_control_slider.svg"},
};

CustomUIModel::CustomUIModel(ParamsModel* params, QObject* parent)
    : QObject(parent)
    , m_params(params)
    , m_tabModel(nullptr)
    , m_primOutputModel(nullptr)
    , m_objInputModel(nullptr)
    , m_objOutputModel(nullptr)
    , m_ctrlItemModel(nullptr)
{
    const zeno::CustomUI& customUI = m_params->customUI();
    m_tabModel = new ParamTabModel(customUI.inputPrims, this);
    m_primOutputModel = new PrimParamOutputModel(customUI.outputPrims, this);
    m_objInputModel = new objParamInputModel(customUI.inputObjs, this);
    m_objOutputModel = new objParamOutputModel(customUI.outputObjs, this);
    m_ctrlItemModel = new ControlItemListModel(this);
}

ParamTabModel* CustomUIModel::tabModel() const
{
    return m_tabModel;
}

PrimParamOutputModel* CustomUIModel::primOutputModel() const
{
    return m_primOutputModel;
}

objParamInputModel* CustomUIModel::objInputModel() const
{
    return m_objInputModel;
}

objParamOutputModel* CustomUIModel::objOutputModel() const
{
    return m_objOutputModel;
}

ParamsModel* CustomUIModel::coreModel() const {
    return m_params;
}

ControlItemListModel* CustomUIModel::controlItemModel() const
{
    return m_ctrlItemModel;
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
                //paramsmodel->insertRow(first, newItem->data(QtRole::ROLE_PARAM_NAME).toString());
            }
        }
    });
    connect(customuiStandarditemModel, &QStandardItemModel::rowsAboutToBeRemoved, [customuiStandarditemModel, this](const QModelIndex& parent, int first, int last) {
        QStandardItem* parentItem = customuiStandarditemModel->itemFromIndex(parent);
        QStandardItem* removeItem = parentItem->child(first);
        int elemtype = removeItem->data(ROLE_ELEMENT_TYPE).toInt();
        ParamTabModel* tabmodel = tabModel();
        if (elemtype == VPARAM_TAB) {
            tabmodel->removeRow(first);
        }
        else if (elemtype == VPARAM_GROUP) {
            ParamGroupModel* group = tabmodel->index(parent.row()).data(QmlCUIRole::GroupModel).value<ParamGroupModel*>();
            group->removeRow(first);
        }
        else if (elemtype == VPARAM_PARAM) {
            QModelIndex& tabindx = parent.parent();
            if (ParamGroupModel* groupmodel = tabmodel->index(tabindx.row()).data(QmlCUIRole::GroupModel).value<ParamGroupModel*>()) {
                ParamPlainModel* paramsmodel = groupmodel->index(parent.row()).data(QmlCUIRole::PrimModel).value<ParamPlainModel*>();
                paramsmodel->removeRow(first);

                tabmodel->setData(tabmodel->index(0,0), "dddd", QtRole::ROLE_PARAM_NAME);
            }
        }
    });
    connect(customuiStandarditemModel, &QStandardItemModel::dataChanged, [customuiStandarditemModel, this](const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles) {
        if (roles.size() == 1 && roles[0] == QtRole::ROLE_PARAM_NAME) {
            QStandardItem* changeItem = customuiStandarditemModel->itemFromIndex(topLeft);
            int elemtype = changeItem->data(ROLE_ELEMENT_TYPE).toInt();
            ParamTabModel* tabmodel = tabModel();
            if (elemtype == VPARAM_TAB) {
                tabmodel->setData(topLeft, topLeft.data(QtRole::ROLE_PARAM_NAME), QtRole::ROLE_PARAM_NAME);
            } else if (elemtype == VPARAM_GROUP) {
                ParamGroupModel* group = tabmodel->index(topLeft.parent().row()).data(QmlCUIRole::GroupModel).value<ParamGroupModel*>();
                group->setData(topLeft, topLeft.data(QtRole::ROLE_PARAM_NAME), QtRole::ROLE_PARAM_NAME);
            } else if (elemtype == VPARAM_PARAM) {
                QModelIndex& tabindx = topLeft.parent().parent();
                if (ParamGroupModel* groupmodel = tabmodel->index(tabindx.row()).data(QmlCUIRole::GroupModel).value<ParamGroupModel*>()) {
                    ParamPlainModel* paramsmodel = groupmodel->index(topLeft.parent().row()).data(QmlCUIRole::PrimModel).value<ParamPlainModel*>();
                    paramsmodel->setData(topLeft, topLeft.data(QtRole::ROLE_PARAM_NAME), QtRole::ROLE_PARAM_NAME);
                }
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
    if (role == QtRole::ROLE_PARAM_NAME) {
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
    if (!index.isValid()) {
        return false;
    }
    switch (role) {
    case QtRole::ROLE_PARAM_NAME:
        m_items[index.row()].name = value.toString();
        emit dataChanged(index, index, { QtRole::ROLE_PARAM_NAME });
        return true;
    }
    return false;
}

QHash<int, QByteArray> ParamTabModel::roleNames() const {
    QHash<int, QByteArray> values;
    values[QtRole::ROLE_PARAM_NAME] = "tabname";
    values[QmlCUIRole::GroupModel] = "groups";
    return values;
}

Qt::ItemFlags ParamTabModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;
    return Qt::ItemIsEditable | QAbstractListModel::flags(index);
}

bool ParamTabModel::insertRow(int row, QString name) {
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
    if (!index.isValid()) {
        return false;
    }
    switch (role) {
    case Qt::DisplayRole:
        m_items[index.row()].name = value.toString();
        emit dataChanged(index, index, { Qt::DisplayRole });
        return true;
    }
    return false;
}

QHash<int, QByteArray> ParamGroupModel::roleNames() const {
    QHash<int, QByteArray> values;
    values[Qt::DisplayRole] = "groupname";
    values[QmlCUIRole::PrimModel] = "params";
    return values;
}


bool ParamGroupModel::insertRow(int row, QString name)
{
    if (row < 0 || row > m_items.size()) {
        return false;
    }

    beginInsertRows(QModelIndex(), row, row);
    _GroupItem item;
    item.name = name;
    item.paramM = new ParamPlainModel(zeno::ParamGroup{ name.toStdString() }, this);
    m_items.insert(row, std::move(item));
    endInsertRows();
    return true;
}

bool ParamGroupModel::removeRow(int row)
{
    if (row < 0 || row >= m_items.size()) {
        return false;
    }

    beginRemoveRows(QModelIndex(), row, row);
    m_items.erase(m_items.begin() + row);
    endRemoveRows();
    return true;
}

////////////////////////////////////////////////////////
ParamPlainModel::ParamPlainModel(zeno::ParamGroup group, ParamGroupModel* pModel)
    : QAbstractListModel(pModel)
    , m_groupModel(pModel)
    , m_paramsModel(nullptr)
{
    m_paramsModel = m_groupModel->tabModel()->uimodel()->coreModel();
    for (const zeno::ParamPrimitive& param : group.params) {
        int r = m_paramsModel->indexFromName(QString::fromStdString(param.name), true);
        QPersistentModelIndex idx = m_paramsModel->index(r);
        m_items.push_back(idx);
    }
}

QString ParamPlainModel::getMaxLengthName() const
{
    QString maxName;
    for (auto& idx : m_items) {
         const QString& name = idx.data(QtRole::ROLE_PARAM_NAME).toString();
         if (name.length() > maxName.length()) {
             maxName = name;
         }
    }
    return maxName;
}

int ParamPlainModel::rowCount(const QModelIndex& parent) const {
    return m_items.size();
}

QVariant ParamPlainModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) {
        return QVariant();
    }
    if (role == Qt::DecorationRole) {
        auto currtype = m_items[index.row()].data(QtRole::ROLE_PARAM_TYPE).toLongLong();
        for (auto& item: customui_controlList) {
            if (item.type == currtype) {
                return item.icon;
            }
        }
    }
    return m_items[index.row()].data(role);
}

bool ParamPlainModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    return m_paramsModel->setData(m_items[index.row()], value, role);
}

QHash<int, QByteArray> ParamPlainModel::roleNames() const {
    //copy from ParamsModel::roleNames()
    QHash<int, QByteArray> roles;
    roles[QtRole::ROLE_NODE_NAME] = "nodename";
    roles[QtRole::ROLE_PARAM_NAME] = "name";
    roles[QtRole::ROLE_PARAM_TYPE] = "type";
    roles[QtRole::ROLE_PARAM_CONTROL] = "control";
    roles[QtRole::ROLE_PARAM_QML_VALUE] = "value";
    roles[QtRole::ROLE_ISINPUT] = "input";
    roles[QtRole::ROLE_PARAM_GROUP] = "group";
    roles[QtRole::ROLE_PARAM_SOCKET_VISIBLE] = "socket_visible";
    roles[QtRole::ROLE_PARAM_CONTROL_PROPS] = "control_properties";
    roles[QtRole::ROLE_PARAM_SOCKET_CLR] = "socket_color";
    roles[QtRole::ROLE_PARAM_PERSISTENT_INDEX] = "per_index";
    roles[QtRole::ROLE_PARAM_VISIBLE] = "param_visible";
    roles[Qt::DecorationRole] = "paramIcon";
    return roles;
}


bool ParamPlainModel::insertRow(int row, QString name, int ctrlItemRow)
{
    if (row < 0 || row > m_items.size()) {
        return false;
    }

    beginInsertRows(QModelIndex(), row, row);
    ParamItem item;
    item.bInput = true;
    item.control = customui_controlList[ctrlItemRow].ctrl;
    if (item.control == zeno::NullControl)
        item.control = zeno::getDefaultControl(customui_controlList[ctrlItemRow].type);
    //item.optCtrlprops = spParam.ctrlProps;
    item.name = name;
    item.type = customui_controlList[ctrlItemRow].type;
    item.value = zeno::initAnyDeflValue(item.type);
    item.connectProp = zeno::Socket_Primitve;
    item.bSocketVisible = true;
    item.bVisible = true;
    item.bEnable = true;
    item.group = zeno::Role_InputPrimitive;
    m_paramsModel->addParam(item);
    int r = m_paramsModel->indexFromName(name, true);
    QPersistentModelIndex idx = m_paramsModel->index(r);
    m_items.insert(row, std::move(idx));
    endInsertRows();
    return true;
}

bool ParamPlainModel::removeRow(int row)
{
    if (row < 0 || row >= m_items.size()) {
        return false;
    }
    //m_paramsModel->removeParam(m_items[row].data(QtRole::ROLE_PARAM_NAME).toString(), true);

    beginRemoveRows(QModelIndex(), row, row);
    m_items.erase(m_items.begin() + row);
    endRemoveRows();
    return true;
}

///////////////////////////////////////////////////////////////
PrimParamOutputModel::PrimParamOutputModel(zeno::PrimitiveParams params, CustomUIModel* pModel)
    : QAbstractListModel(pModel)
    , m_paramsModel(nullptr)
{
    m_paramsModel = pModel->coreModel();
    for (const zeno::ParamPrimitive& param : params) {
        int r = m_paramsModel->indexFromName(QString::fromStdString(param.name), false);
        QPersistentModelIndex idx = m_paramsModel->index(r);
        m_items.push_back(idx);
    }
}

int PrimParamOutputModel::rowCount(const QModelIndex& parent) const {
    return m_items.size();
}

QVariant PrimParamOutputModel::data(const QModelIndex& index, int role) const {
    return m_items[index.row()].data(role);
}

bool PrimParamOutputModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    return m_paramsModel->setData(m_items[index.row()], value, role);
}

QHash<int, QByteArray> PrimParamOutputModel::roleNames() const {
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

Q_INVOKABLE bool PrimParamOutputModel::insertRow(int row, QString name)
{
    if (row < 0 || row > m_items.size()) {
        return false;
    }

    beginInsertRows(QModelIndex(), row, row);
    ParamItem item;
    item.bInput = false;
    item.control = zeno::NullControl;
    //item.optCtrlprops = spParam.ctrlProps;
    item.name = name;
    item.type = Param_Wildcard;
    item.value = zeno::initAnyDeflValue(item.type);
    item.connectProp = zeno::Socket_Primitve;
    item.bSocketVisible = true;
    item.bVisible = true;
    item.bEnable = true;
    item.group = zeno::Role_OutputPrimitive;
    //m_paramsModel->addParam(item);
    int r = m_paramsModel->indexFromName(name, false);
    QPersistentModelIndex idx = m_paramsModel->index(r);
    m_items.insert(row, std::move(idx));
    endInsertRows();
    return true;
}

Q_INVOKABLE bool PrimParamOutputModel::removeRow(int row)
{
    if (row < 0 || row >= m_items.size()) {
        return false;
    }
    m_paramsModel->removeParam(m_items[row].data(QtRole::ROLE_PARAM_NAME).toString(), false);

    beginRemoveRows(QModelIndex(), row, row);
    m_items.erase(m_items.begin() + row);
    endRemoveRows();
    return true;
}

objParamInputModel::objParamInputModel(zeno::ObjectParams params, CustomUIModel* pModel)
    : QAbstractListModel(pModel)
    , m_paramsModel(nullptr)
{
    m_paramsModel = pModel->coreModel();
    for (const zeno::ParamObject& param : params) {
        int r = m_paramsModel->indexFromName(QString::fromStdString(param.name), true);
        QPersistentModelIndex idx = m_paramsModel->index(r);
        m_items.push_back(idx);
    }
}

int objParamInputModel::rowCount(const QModelIndex& parent /*= QModelIndex()*/) const
{
    return m_items.size();
}

QVariant objParamInputModel::data(const QModelIndex& index, int role /*= Qt::DisplayRole*/) const
{
    return m_items[index.row()].data(role);
}

bool objParamInputModel::setData(const QModelIndex& index, const QVariant& value, int role /*= Qt::EditRole*/)
{
    return m_paramsModel->setData(m_items[index.row()], value, role);
}

QHash<int, QByteArray> objParamInputModel::roleNames() const
{
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

Q_INVOKABLE bool objParamInputModel::insertRow(int row, QString name)
{
    if (row < 0 || row > m_items.size()) {
        return false;
    }

    beginInsertRows(QModelIndex(), row, row);
    ParamItem item;
    item.bInput = true;
    item.control = zeno::NullControl;
    //item.optCtrlprops = spParam.ctrlProps;
    item.name = name;
    item.type = Obj_Wildcard;
    item.value = zeno::initAnyDeflValue(item.type);
    item.connectProp = zeno::Socket_Output;
    item.bSocketVisible = true;
    item.bVisible = true;
    item.bEnable = true;
    item.group = zeno::Role_InputObject;
    //m_paramsModel->addParam(item);
    int r = m_paramsModel->indexFromName(name, true);
    QPersistentModelIndex idx = m_paramsModel->index(r);
    m_items.insert(row, std::move(idx));
    endInsertRows();
    return true;
}

Q_INVOKABLE bool objParamInputModel::removeRow(int row)
{
    if (row < 0 || row >= m_items.size()) {
        return false;
    }
    m_paramsModel->removeParam(m_items[row].data(QtRole::ROLE_PARAM_NAME).toString(), true);

    beginRemoveRows(QModelIndex(), row, row);
    m_items.erase(m_items.begin() + row);
    endRemoveRows();
    return true;
}

objParamOutputModel::objParamOutputModel(zeno::ObjectParams params, CustomUIModel* pModel)
    : QAbstractListModel(pModel)
    , m_paramsModel(nullptr)
{
    m_paramsModel = pModel->coreModel();
    for (const zeno::ParamObject& param : params) {
        int r = m_paramsModel->indexFromName(QString::fromStdString(param.name), false);
        QPersistentModelIndex idx = m_paramsModel->index(r);
        m_items.push_back(idx);
    }
}

int objParamOutputModel::rowCount(const QModelIndex& parent /*= QModelIndex()*/) const
{
    return m_items.size();
}

QVariant objParamOutputModel::data(const QModelIndex& index, int role /*= Qt::DisplayRole*/) const
{
    return m_items[index.row()].data(role);
}

bool objParamOutputModel::setData(const QModelIndex& index, const QVariant& value, int role /*= Qt::EditRole*/)
{
    return m_paramsModel->setData(m_items[index.row()], value, role);
}

QHash<int, QByteArray> objParamOutputModel::roleNames() const
{
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

Q_INVOKABLE bool objParamOutputModel::insertRow(int row, QString name)
{
    if (row < 0 || row > m_items.size()) {
        return false;
    }

    beginInsertRows(QModelIndex(), row, row);
    ParamItem item;
    item.bInput = false;
    item.control = zeno::NullControl;
    //item.optCtrlprops = spParam.ctrlProps;
    item.name = name;
    item.type = Obj_Wildcard;
    item.value = zeno::initAnyDeflValue(item.type);
    item.connectProp = zeno::Socket_Output;
    item.bSocketVisible = true;
    item.bVisible = true;
    item.bEnable = true;
    item.group = zeno::Role_InputObject;
    //m_paramsModel->addParam(item);
    int r = m_paramsModel->indexFromName(name, false);
    QPersistentModelIndex idx = m_paramsModel->index(r);
    m_items.insert(row, std::move(idx));
    endInsertRows();
    return true;
}


Q_INVOKABLE bool objParamOutputModel::removeRow(int row)
{
    if (row < 0 || row >= m_items.size()) {
        return false;
    }
    m_paramsModel->removeParam(m_items[row].data(QtRole::ROLE_PARAM_NAME).toString(), false);

    beginRemoveRows(QModelIndex(), row, row);
    m_items.erase(m_items.begin() + row);
    endRemoveRows();
    return true;
}

ControlItemListModel::ControlItemListModel(CustomUIModel* pModel) : QAbstractListModel(pModel)
{
}

int ControlItemListModel::rowCount(const QModelIndex& parent) const
{
    return sizeof(customui_controlList) / sizeof(CUSTOMUI_CTRL_ITEM_INFO);
}

QVariant ControlItemListModel::data(const QModelIndex& index, int role) const
{
    if (role == QtRole::ROLE_PARAM_NAME) {
        return customui_controlList[index.row()].name;
    } else if (role == QtRole::ROLE_PARAM_CONTROL) {
        return customui_controlList[index.row()].ctrl;
    } else if (role == QtRole::ROLE_PARAM_TYPE) {
        return customui_controlList[index.row()].type;
    } else if (role == Qt::DecorationRole) {
        return customui_controlList[index.row()].icon;
    }
    return QVariant();
}

QHash<int, QByteArray> ControlItemListModel::roleNames() const
{
    QHash<int, QByteArray> values;
    values[QtRole::ROLE_PARAM_NAME] = "ctrlItemName";
    values[QtRole::ROLE_PARAM_CONTROL] = "ctrlItemType";
    values[QtRole::ROLE_PARAM_TYPE] = "ctrlContentType";
    values[Qt::DecorationRole] = "ctrlItemICon";
    return values;
}

Qt::ItemFlags ControlItemListModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}
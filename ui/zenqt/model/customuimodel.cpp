#include "customuimodel.h"
#include <zeno/utils/helper.h>
#include "style/colormanager.h"
#include "util/uihelper.h"
#include "zeno_types/reflect/reflection.generated.hpp"
#include "declmetatype.h"

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

void appendClonedItem(QVector<ParamItem>& parasm, ParamsModel* m_paramsModel, QModelIndex& idx) {
    ParamItem item;
    item.bInput = m_paramsModel->data(idx, QtRole::ROLE_ISINPUT).toBool();
    item.control = (zeno::ParamControl)m_paramsModel->data(idx, QtRole::ROLE_PARAM_CONTROL).toInt();
    item.optCtrlprops = m_paramsModel->data(idx, QtRole::ROLE_PARAM_CTRL_PROPERTIES).value<zeno::reflect::Any>();
    item.name = m_paramsModel->data(idx, QtRole::ROLE_PARAM_NAME).toString();
    item.type = (zeno::ParamType)m_paramsModel->data(idx, QtRole::ROLE_PARAM_TYPE).toLongLong();
    item.value = m_paramsModel->data(idx, QtRole::ROLE_PARAM_VALUE).value<zeno::reflect::Any>();
    item.connectProp = (zeno::SocketType)m_paramsModel->data(idx, QtRole::ROLE_SOCKET_TYPE).toInt();
    item.bSocketVisible = m_paramsModel->data(idx, QtRole::ROLE_PARAM_SOCKET_VISIBLE).toBool();
    item.bVisible = m_paramsModel->data(idx, QtRole::ROLE_PARAM_VISIBLE).toBool();
    item.bEnable = m_paramsModel->data(idx, QtRole::ROLE_PARAM_ENABLE).toBool();
    item.group = (zeno::NodeDataGroup)m_paramsModel->data(idx, QtRole::ROLE_PARAM_GROUP).toInt();
    item.sockProp = (zeno::SocketProperty)m_paramsModel->data(idx, QtRole::ROLE_PARAM_SOCKPROP).toUInt();
    parasm.append(item);
}

QVariant cloneItemGetData(const QVector<ParamItem>& clonedItems, const QModelIndex& index, int role) {
    if (!index.isValid()) {
        return QVariant();
    }
    if (role == Qt::DecorationRole) {
        for (auto& item : customui_controlList) {
            if (item.ctrl == clonedItems[index.row()].control && item.type == clonedItems[index.row()].type) { return item.icon; }
        }
    }
    else if (role == QtRole::ROLE_NODE_NAME) {
        return clonedItems[index.row()].name;
    }
    else if (role == QtRole::ROLE_PARAM_NAME) {
        return clonedItems[index.row()].name;
    }
    else if (role == QtRole::ROLE_PARAM_TYPE) {
        return clonedItems[index.row()].type;
    }
    else if (role == QtRole::ROLE_PARAM_CONTROL) {
        return clonedItems[index.row()].control;
    }
    else if (role == QtRole::ROLE_PARAM_QML_VALUE) {
        return QVariant::fromValue(clonedItems[index.row()].value);
    }
    else if (role == QtRole::ROLE_ISINPUT) {
        return clonedItems[index.row()].bInput;
    }
    else if (role == QtRole::ROLE_PARAM_GROUP) {
        return clonedItems[index.row()].group;
    }
    else if (role == QtRole::ROLE_PARAM_SOCKET_VISIBLE) {
        return clonedItems[index.row()].bSocketVisible;
    }
    else if (role == QtRole::ROLE_PARAM_CONTROL_PROPS) {
        QVariantMap map;
        if (clonedItems[index.row()].optCtrlprops.has_value()) {
            if (clonedItems[index.row()].optCtrlprops.type().hash_code() == zeno::types::gParamType_StringList) {
                const auto& items = zeno::reflect::any_cast<std::vector<std::string>>(clonedItems[index.row()].optCtrlprops);
                QStringList qitems;
                for (const auto& item : items) { qitems.append(QString::fromStdString(item)); }
                map["combobox_items"] = qitems;
            }
            if (clonedItems[index.row()].optCtrlprops.type().hash_code() == zeno::types::gParamType_IntList) {
                const auto& items = zeno::reflect::any_cast<std::vector<int>>(clonedItems[index.row()].optCtrlprops);
                QVariantList qitems;
                for (int item : items) { qitems.append(item); }
                map["slider"] = qitems;
            }
        }
        return map;
    }
    else if (role == QtRole::ROLE_PARAM_SOCKET_CLR) {
        QColor color = ZColorManager::getColorByType(clonedItems[index.row()].type);
        return color;
    }
    else if (role == QtRole::ROLE_PARAM_PERSISTENT_INDEX) {
        return false;
    }
    else {
        return QVariant();
    }
}

bool cloneItemSetdata(QVector<ParamItem>& clonedItems, const QModelIndex& index, const QVariant& value, int role) {
    ParamItem& param = clonedItems[index.row()];
    switch (role) {
    case QtRole::ROLE_PARAM_NAME:
        param.name = value.toString();
        break;
    case QtRole::ROLE_PARAM_TYPE:
        param.type = (zeno::ParamType)value.toLongLong();
        break;
    case QtRole::ROLE_PARAM_QML_VALUE: {
        //在QML初始化的时候，比如onValueChanged时也会直接调用，要考虑是不是做同值过滤
        const zeno::reflect::Any& anyVal = UiHelper::qvarToAnyByType(value, param.type, param.control == zeno::Lineedit);
        param.value = anyVal;
        break;
    }
    case QtRole::ROLE_PARAM_VALUE: {
        const zeno::reflect::Any& anyVal = value.value<zeno::reflect::Any>();
        if (anyVal == param.value) {
            return false;
        }
        param.value = anyVal;
        return false;
    }

    case QtRole::ROLE_PARAM_CONTROL:
        param.control = (zeno::ParamControl)value.toInt();
        break;
    case QtRole::ROLE_PARAM_CTRL_PROPERTIES:
        param.optCtrlprops = value.value<zeno::reflect::Any>();
    case QtRole::ROLE_SOCKET_TYPE:
        param.connectProp = (zeno::SocketType)value.toInt();
        break;
    case QtRole::ROLE_PARAM_IS_WILDCARD:
        param.bWildcard = value.toBool();
        break;
    case QtRole::ROLE_PARAM_SOCKET_VISIBLE:
        param.bSocketVisible = value.toBool();
        break;
    case QtRole::ROLE_PARAM_PERSISTENT_INDEX:
        return false;
    }
    return true;
}

CustomUIModel::CustomUIModel(ParamsModel* params, QObject* parent, bool isCloneCustomuiModel)
    : QObject(parent)
    , m_params(params)
    , m_tabModel(nullptr)
    , m_primOutputModel(nullptr)
    , m_objInputModel(nullptr)
    , m_objOutputModel(nullptr)
    , m_ctrlItemModel(nullptr)
    , m_bIsCloneCustomuiModel(isCloneCustomuiModel)
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

Q_INVOKABLE void CustomUIModel::reset()
{
    if (!m_bIsCloneCustomuiModel) {
        m_tabModel->reset();
        m_primOutputModel->reset();
        m_objInputModel->reset();
        m_objOutputModel->reset();
    }
}

bool CustomUIModel::isSubnetNode()
{
    if (m_params->rowCount() != 0) {
        auto idx = m_params->index(0);
        QModelIndex nodeidx = m_params->data(idx, QtRole::ROLE_NODEIDX).value<QModelIndex>();
        if (nodeidx.isValid() && (zeno::NodeType)nodeidx.data(QtRole::ROLE_NODETYPE).toInt() == zeno::Node_SubgraphNode) {
            return true;
        }
    }
    return false;
}

bool CustomUIModel::isClonedModel()
{
    return m_bIsCloneCustomuiModel;
}

void CustomUIModel::exportCustomuiAndEdittedUpdateInfo(zeno::CustomUI& customui, zeno::ParamsUpdateInfo& editUpdateInfo)
{
    m_tabModel->exportCustomuiAndEdittedUpdateInfo(customui, editUpdateInfo);
    m_primOutputModel->exportCustomuiAndEdittedUpdateInfo(customui, editUpdateInfo);
    m_objInputModel->exportCustomuiAndEdittedUpdateInfo(customui, editUpdateInfo);
    m_objOutputModel->exportCustomuiAndEdittedUpdateInfo(customui, editUpdateInfo);
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

void ParamTabModel::reset()
{
    beginResetModel();
    for (auto& item : m_items) {
        delete item.groupM;
    }
    m_items.clear();
    const zeno::CustomUI& customUI = m_customuiM->coreModel()->customUI();
    for (const zeno::ParamTab& tab : customUI.inputPrims) {
        _TabItem item;
        item.name = QString::fromStdString(tab.name);
        item.groupM = new ParamGroupModel(tab, this);
        m_items.push_back(std::move(item));
    }
    endResetModel();
}

void ParamTabModel::exportCustomuiAndEdittedUpdateInfo(zeno::CustomUI& customui, zeno::ParamsUpdateInfo& editUpdateInfo)
{
    for (auto& tabitem : m_items) {
        zeno::ParamTab tabInfo;
        tabInfo.name = tabitem.name.toStdString();
        std::vector<zeno::ParamGroup> groupinfo;
        tabitem.groupM->exportCustomuiAndEdittedUpdateInfo(groupinfo, editUpdateInfo);
        tabInfo.groups = groupinfo;
        customui.inputPrims.push_back(tabInfo);
    }
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

void ParamGroupModel::exportCustomuiAndEdittedUpdateInfo(std::vector<zeno::ParamGroup>& group, zeno::ParamsUpdateInfo& editUpdateInfo)
{
    for (auto& groupitem : m_items) {
        zeno::ParamGroup groupinfo;
        groupinfo.name = groupitem.name.toStdString();
        std::vector<zeno::ParamPrimitive> params;
        groupitem.paramM->exportCustomuiAndEdittedUpdateInfo(params, editUpdateInfo);
        groupinfo.params = params;
        group.push_back(groupinfo);
    }
}

////////////////////////////////////////////////////////
ParamPlainModel::ParamPlainModel(zeno::ParamGroup group, ParamGroupModel* pModel)
    : QAbstractListModel(pModel)
    , m_groupModel(pModel)
    , m_paramsModel(nullptr)
    , m_bIscloned(m_groupModel->tabModel()->uimodel()->isClonedModel())
{
    m_paramsModel = m_groupModel->tabModel()->uimodel()->coreModel();
    if (m_bIscloned) {
        for (const zeno::ParamPrimitive& param : group.params) {
            int r = m_paramsModel->indexFromName(QString::fromStdString(param.name), true);
            auto idx = m_paramsModel->index(r);
            appendClonedItem(m_clonedItems, m_paramsModel, idx);
        }
    }
    else {
        for (const zeno::ParamPrimitive& param : group.params) {
            int r = m_paramsModel->indexFromName(QString::fromStdString(param.name), true);
            QPersistentModelIndex idx = m_paramsModel->index(r);
            m_items.push_back(idx);
        }
    }
}

QString ParamPlainModel::getMaxLengthName() const
{
    QString maxName;
    if (m_bIscloned) {
        for (auto& item : m_clonedItems) {
            const QString& name = item.name;
            if (name.length() > maxName.length()) {
                maxName = name;
            }
        }
    }
    else {
        for (auto& idx : m_items) {
             const QString& name = idx.data(QtRole::ROLE_PARAM_NAME).toString();
             if (name.length() > maxName.length()) {
                 maxName = name;
             }
        }
    }
    return maxName;
}

int ParamPlainModel::rowCount(const QModelIndex& parent) const {
    if (m_bIscloned) {
        m_clonedItems.size();
    }
    else {
        return m_items.size();
    }
}

QVariant ParamPlainModel::data(const QModelIndex& index, int role) const {
    if (m_bIscloned) {
        return cloneItemGetData(m_clonedItems, index, role);
    }
    else {
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
}

bool ParamPlainModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    if (m_bIscloned) {
        return cloneItemSetdata(m_clonedItems, index, value, role);
    } else {
        return m_paramsModel->setData(m_items[index.row()], value, role);
    }
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
    if (m_bIscloned) {
        if (row < 0 || row > m_clonedItems.size()) {
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
        m_clonedItems.insert(row, std::move(item));
        endInsertRows();
        return true;
    }
    return false;
}

bool ParamPlainModel::removeRow(int row)
{
    if (m_bIscloned) {
        if (row < 0 || row >= m_clonedItems.size()) {
            return false;
        }
        beginRemoveRows(QModelIndex(), row, row);
        m_clonedItems.erase(m_clonedItems.begin() + row);
        endRemoveRows();
        return true;
    }
    return false;
}

void ParamPlainModel::exportCustomuiAndEdittedUpdateInfo(std::vector<zeno::ParamPrimitive>& params, zeno::ParamsUpdateInfo& editUpdateInfo)
{
    if (m_bIscloned) {
        for (auto& paramitem : m_clonedItems) {
            zeno::ParamPrimitive param;
            param.name = paramitem.name.toStdString();
            param.defl = paramitem.value;
            param.control = paramitem.control;
            param.type = paramitem.type;
            //lineEdit组件的值类型设为gParamType_PrimVariant（和ParamsModel::setData类型为QtRole::ROLE_PARAM_QML_VALUE的数据时的判断保持一致）
            //param.type = paramitem.control == zeno::Lineedit ? zeno::types::gParamType_PrimVariant : paramitem.type;
            param.socketType = paramitem.connectProp;
            param.ctrlProps = paramitem.optCtrlprops;
            params.push_back(param);

            int r = m_paramsModel->indexFromName(QString::fromStdString(param.name), true);
            editUpdateInfo.push_back({ param, r == -1 ? "" : param.name });
        }
    }
}


///////////////////////////////////////////////////////////////
PrimParamOutputModel::PrimParamOutputModel(zeno::PrimitiveParams params, CustomUIModel* pModel)
    : QAbstractListModel(pModel)
    , m_paramsModel(nullptr)
    , m_bIscloned(pModel->isClonedModel())
{
    m_paramsModel = pModel->coreModel();
    for (const zeno::ParamPrimitive& param : params) {
        int r = m_paramsModel->indexFromName(QString::fromStdString(param.name), false);
        QPersistentModelIndex idx = m_paramsModel->index(r);
        if (m_bIscloned) {
            appendClonedItem(m_clonedItems, m_paramsModel, m_paramsModel->index(r));
        }
        else {
            m_items.push_back(idx);
        }
    }
}

int PrimParamOutputModel::rowCount(const QModelIndex& parent) const {
    if (m_bIscloned) {
        m_clonedItems.size();
    }
    else {
        return m_items.size();
    }
}

QVariant PrimParamOutputModel::data(const QModelIndex& index, int role) const {
    if (m_bIscloned) {
        return cloneItemGetData(m_clonedItems, index, role);
    }
    else {
        return m_items[index.row()].data(role);
    }
}

bool PrimParamOutputModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    if (m_bIscloned) {
        return cloneItemSetdata(m_clonedItems, index, value, role);
    }
    else {
        return m_paramsModel->setData(m_items[index.row()], value, role);
    }
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
    if (m_bIscloned) {
        if (row < 0 || row > m_clonedItems.size()) {
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
        m_clonedItems.insert(row, item);
        endInsertRows();
        return true;
    }
}

Q_INVOKABLE bool PrimParamOutputModel::removeRow(int row)
{
    if (m_bIscloned) {
         if (row < 0 || row >= m_clonedItems.size()) {
            return false;
        }
        beginRemoveRows(QModelIndex(), row, row);
        m_clonedItems.erase(m_clonedItems.begin() + row);
        endRemoveRows();
        return true;
    }
}

void PrimParamOutputModel::reset()
{
    beginResetModel();
    m_items.clear();
    const zeno::CustomUI& customUI = m_paramsModel->customUI();
    for (const zeno::ParamPrimitive& param : customUI.outputPrims) {
        int r = m_paramsModel->indexFromName(QString::fromStdString(param.name), false);
        QPersistentModelIndex idx = m_paramsModel->index(r);
        m_items.push_back(idx);
    }
    endResetModel();
}

void PrimParamOutputModel::exportCustomuiAndEdittedUpdateInfo(zeno::CustomUI& customui, zeno::ParamsUpdateInfo& editUpdateInfo)
{
    if (m_bIscloned) {
        for (auto& paramitem : m_clonedItems) {
            zeno::ParamPrimitive param;
            param.bInput = false;
            param.control = paramitem.control;
            param.type = paramitem.type;
            param.defl = paramitem.value;
            param.name = paramitem.name.toStdString();
            param.socketType = paramitem.connectProp;
            param.ctrlProps = paramitem.optCtrlprops;
            customui.outputPrims.push_back(param);

            int r = m_paramsModel->indexFromName(QString::fromStdString(param.name), false);
            editUpdateInfo.push_back({ param, r == -1 ? "" : param.name });
        }
    }
}

objParamInputModel::objParamInputModel(zeno::ObjectParams params, CustomUIModel* pModel)
    : QAbstractListModel(pModel)
    , m_paramsModel(nullptr)
    , m_bIscloned(pModel->isClonedModel())
{
    m_paramsModel = pModel->coreModel();
    for (const zeno::ParamObject& param : params) {
        int r = m_paramsModel->indexFromName(QString::fromStdString(param.name), true);
        QPersistentModelIndex idx = m_paramsModel->index(r);
        if (m_bIscloned) {
            appendClonedItem(m_clonedItems, m_paramsModel, m_paramsModel->index(r));
        }
        else {
            m_items.push_back(idx);
        }
    }
}


int objParamInputModel::rowCount(const QModelIndex& parent /*= QModelIndex()*/) const
{
    if (m_bIscloned) {
        m_clonedItems.size();
    }
    else {
        return m_items.size();
    }
}

QVariant objParamInputModel::data(const QModelIndex& index, int role /*= Qt::DisplayRole*/) const
{
    if (m_bIscloned) {
        return cloneItemGetData(m_clonedItems, index, role);
    }
    else {
        return m_items[index.row()].data(role);
    }
}

bool objParamInputModel::setData(const QModelIndex& index, const QVariant& value, int role /*= Qt::EditRole*/)
{
    if (m_bIscloned) {
        return cloneItemSetdata(m_clonedItems, index, value, role);
    }
    else {
        return m_paramsModel->setData(m_items[index.row()], value, role);
    }
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
    if (m_bIscloned) {
        if (row < 0 || row > m_clonedItems.size()) {
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
        m_clonedItems.insert(row, std::move(item));
        endInsertRows();
        return true;
    }
}

Q_INVOKABLE bool objParamInputModel::removeRow(int row)
{
    if (m_bIscloned) {
        if (row < 0 || row >= m_clonedItems.size()) {
            return false;
        }
        beginRemoveRows(QModelIndex(), row, row);
        m_clonedItems.erase(m_clonedItems.begin() + row);
        endRemoveRows();
        return true;
    }
}

void objParamInputModel::reset()
{
    beginResetModel();
    m_items.clear();
    const zeno::CustomUI& customUI = m_paramsModel->customUI();
    for (const zeno::ParamObject& param : customUI.inputObjs) {
        int r = m_paramsModel->indexFromName(QString::fromStdString(param.name), true);
        QPersistentModelIndex idx = m_paramsModel->index(r);
        m_items.push_back(idx);
    }
    endResetModel();
}

void objParamInputModel::exportCustomuiAndEdittedUpdateInfo(zeno::CustomUI& customui, zeno::ParamsUpdateInfo& editUpdateInfo)
{
    if (m_bIscloned) {
        for (auto& paramitem : m_clonedItems) {
            zeno::ParamObject param;
            param.bInput = true;
            param.type = paramitem.type;
            param.name = paramitem.name.toStdString();
            param.socketType = paramitem.connectProp;
            customui.inputObjs.push_back(param);

            int r = m_paramsModel->indexFromName(QString::fromStdString(param.name), true);
            editUpdateInfo.push_back({ param, r == -1 ? "" : param.name });
        }
    }
}

objParamOutputModel::objParamOutputModel(zeno::ObjectParams params, CustomUIModel* pModel)
    : QAbstractListModel(pModel)
    , m_paramsModel(nullptr)
    , m_bIscloned(pModel->isClonedModel())
{
    m_paramsModel = pModel->coreModel();
    for (const zeno::ParamObject& param : params) {
        int r = m_paramsModel->indexFromName(QString::fromStdString(param.name), false);
        QPersistentModelIndex idx = m_paramsModel->index(r);
        if (m_bIscloned) {
            appendClonedItem(m_clonedItems, m_paramsModel, m_paramsModel->index(r));
        }
        else {
            m_items.push_back(idx);
        }
    }
}

int objParamOutputModel::rowCount(const QModelIndex& parent /*= QModelIndex()*/) const
{
    if (m_bIscloned) {
        m_clonedItems.size();
    }
    else {
        return m_items.size();
    }
}

QVariant objParamOutputModel::data(const QModelIndex& index, int role /*= Qt::DisplayRole*/) const
{
    if (m_bIscloned) {
        return cloneItemGetData(m_clonedItems, index, role);
    }
    else {
        return m_items[index.row()].data(role);
    }
}

bool objParamOutputModel::setData(const QModelIndex& index, const QVariant& value, int role /*= Qt::EditRole*/)
{
    if (m_bIscloned) {
        return cloneItemSetdata(m_clonedItems, index, value, role);
    }
    else {
        return m_paramsModel->setData(m_items[index.row()], value, role);
    }
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
    if (m_bIscloned) {
        if (row < 0 || row > m_clonedItems.size()) {
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
        m_clonedItems.insert(row, std::move(item));
        endInsertRows();
        return true;
    }
}


Q_INVOKABLE bool objParamOutputModel::removeRow(int row)
{
    if (m_bIscloned) {
        if (row < 0 || row >= m_clonedItems.size()) {
            return false;
        }
        beginRemoveRows(QModelIndex(), row, row);
        m_clonedItems.erase(m_clonedItems.begin() + row);
        endRemoveRows();
        return true;
    }
}

void objParamOutputModel::reset()
{
    beginResetModel();
    m_items.clear();
    const zeno::CustomUI& customUI = m_paramsModel->customUI();
    for (const zeno::ParamObject& param : customUI.outputObjs) {
        int r = m_paramsModel->indexFromName(QString::fromStdString(param.name), false);
        QPersistentModelIndex idx = m_paramsModel->index(r);
        m_items.push_back(idx);
    }
    endResetModel();
}

void objParamOutputModel::exportCustomuiAndEdittedUpdateInfo(zeno::CustomUI& customui, zeno::ParamsUpdateInfo& editUpdateInfo)
{
    if (m_bIscloned) {
        for (auto& paramitem : m_clonedItems) {
            zeno::ParamObject param;
            param.bInput = false;
            param.type = paramitem.type;
            param.name = paramitem.name.toStdString();
            param.socketType = paramitem.connectProp;
            customui.outputObjs.push_back(param);

            int r = m_paramsModel->indexFromName(QString::fromStdString(param.name), false);
            editUpdateInfo.push_back({ param, r == -1 ? "" : param.name });
        }
    }
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
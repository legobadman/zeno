#pragma once

#ifndef __CUSTOMUI_MODEL_H__
#define __CUSTOMUI_MODEL_H__

#include <QObject>
#include <QAbstractItemModel>
#include <QAbstractListModel>
#include <model/parammodel.h>

class ParamTabModel;
class ParamGroupModel;
class ParamPlainModel;
class PrimParamOutputModel;
class objParamInputModel;
class objParamOutputModel;
class ControlItemListModel;


class QmlCUIRole
{
    Q_GADGET
public:
    explicit QmlCUIRole() {}

    enum Value {
        TabModel = Qt::UserRole + 1,
        GroupModel,
        PrimModel,
        OutputModel,
    };
    Q_ENUM(Value)
};


class CustomUIModel : public QObject {
    Q_OBJECT
public:
    CustomUIModel(ParamsModel* params, QObject* parent = nullptr, bool isCloneCustomuiModel = false);
    Q_INVOKABLE ParamTabModel* tabModel() const;
    Q_INVOKABLE PrimParamOutputModel* primOutputModel() const;
    Q_INVOKABLE objParamInputModel* objInputModel() const;
    Q_INVOKABLE objParamOutputModel* objOutputModel() const;
    Q_INVOKABLE ParamsModel* coreModel() const;
    Q_INVOKABLE ControlItemListModel* controlItemModel() const;
    void initCustomuiConnections(QStandardItemModel* customuiStandarditemModel);

    Q_INVOKABLE void reset();
    Q_INVOKABLE bool isSubnetNode();
    bool isClonedModel();
    //clone的情况下，导出customui和editupdateinfo，给parammodel重设参数
    void exportCustomuiAndEdittedUpdateInfo(zeno::CustomUI& customui, zeno::ParamsUpdateInfo& editUpdateInfo);
    void updateModelIncremental(const zeno::params_change_info& params, const zeno::CustomUI& customui);

private:
    ParamsModel* m_params;
    ParamTabModel* m_tabModel;
    PrimParamOutputModel* m_primOutputModel;
    objParamInputModel* m_objInputModel;
    objParamOutputModel* m_objOutputModel;
    ControlItemListModel* m_ctrlItemModel;

    bool m_bIsCloneCustomuiModel;
};


struct CUSTOMUI_CTRL_ITEM_INFO
{
    QString name;
    zeno::ParamControl ctrl;
    zeno::ParamType type;
    QString icon;
};
class ControlItemListModel : public QAbstractListModel
{
    Q_OBJECT
        typedef QAbstractListModel _base;

public:
    ControlItemListModel(CustomUIModel* pModel);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
};


class ParamTabModel : public QAbstractListModel
{
    Q_OBJECT
    typedef QAbstractListModel _base;

    struct _TabItem {
        QString name;
        ParamGroupModel* groupM;
    };

public:
    ParamTabModel(zeno::CustomUIParams tabs, CustomUIModel* pModel);
    Q_INVOKABLE CustomUIModel* uimodel() { return m_customuiM; }

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    QHash<int, QByteArray> roleNames() const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    
    QStandardItemModel* toStandardModel() const;

    Q_INVOKABLE bool insertRow(int row, QString name);
    Q_INVOKABLE bool removeRow(int row);

    void reset();
    void exportCustomuiAndEdittedUpdateInfo(zeno::CustomUI& customui, zeno::ParamsUpdateInfo& editUpdateInfo);

private:
    QVector<_TabItem> m_items;
    CustomUIModel* m_customuiM;
};


class ParamGroupModel : public QAbstractListModel
{
    Q_OBJECT
    typedef QAbstractListModel _base;

    struct _GroupItem {
        QString name;
        ParamPlainModel* paramM;    //一个group下所有的参数
    };

public:
    ParamGroupModel(zeno::ParamTab tab, ParamTabModel* pModel);
    Q_INVOKABLE ParamTabModel* tabModel() { return m_tabModel; }

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE bool insertRow(int row, QString name);
    Q_INVOKABLE bool removeRow(int row);

    void exportCustomuiAndEdittedUpdateInfo(std::vector<zeno::ParamGroup>& group, zeno::ParamsUpdateInfo& editUpdateInfo);

private:
    QVector<_GroupItem> m_items;
    ParamTabModel* m_tabModel;
};


class ParamPlainModel : public QAbstractListModel
{
    Q_OBJECT
    typedef QAbstractListModel _base;

    struct _ParamItem {
        QPersistentModelIndex m_index;  //指向ParamsModel真正储存的参数
    };

public:
    ParamPlainModel(zeno::ParamGroup group, ParamGroupModel* pModel);

    Q_PROPERTY(QString maxLengthName READ getMaxLengthName NOTIFY maxLengthName_changed)

    Q_INVOKABLE ParamGroupModel* groupModel() { return m_groupModel; }

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE bool insertRow(int row, QString name, int ctrlItemRow);
    Q_INVOKABLE bool removeRow(int row);

    void exportCustomuiAndEdittedUpdateInfo(std::vector<zeno::ParamPrimitive>& params, zeno::ParamsUpdateInfo& editUpdateInfo);
signals:
    void maxLengthName_changed();

private:
    QString getMaxLengthName() const;

    QVector<QPersistentModelIndex> m_items;    //一个group下所有的param
    ParamGroupModel* m_groupModel;
    ParamsModel* m_paramsModel;

    bool m_bIscloned;
    QVector<ParamItem> m_clonedItems;//如果customuiModel是以克隆的方式创建，就缓存所有的param，编辑时修改这个变量。
};


class PrimParamOutputModel : public QAbstractListModel
{
    Q_OBJECT
    typedef QAbstractListModel _base;

    struct _ParamItem {
        QPersistentModelIndex m_index;  //指向ParamsModel真正储存的参数
    };

public:
    PrimParamOutputModel(zeno::PrimitiveParams params, CustomUIModel* pModel);

    Q_PROPERTY(QString maxLengthName READ getMaxLengthName NOTIFY maxLengthName_changed)

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    QHash<int, QByteArray> roleNames() const override;
    QStandardItemModel* toStandardModel() const;

    Q_INVOKABLE bool insertRow(int row, QString name);
    Q_INVOKABLE bool removeRow(int row);

    void reset();
    void exportCustomuiAndEdittedUpdateInfo(zeno::CustomUI& customui, zeno::ParamsUpdateInfo& editUpdateInfo);

signals:
    void maxLengthName_changed();

private:
    QString getMaxLengthName() const;

    QVector<QPersistentModelIndex> m_items;    //一个group下所有的param
    ParamsModel* m_paramsModel;

    bool m_bIscloned;
    QVector<ParamItem> m_clonedItems;
};

class objParamInputModel : public QAbstractListModel
{
    Q_OBJECT
        typedef QAbstractListModel _base;

    struct _ParamItem {
        QPersistentModelIndex m_index;  //指向ParamsModel真正储存的参数
    };

public:
    objParamInputModel(zeno::ObjectParams params, CustomUIModel* pModel);
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE bool insertRow(int row, QString name);
    Q_INVOKABLE bool removeRow(int row);

    void reset();
    void exportCustomuiAndEdittedUpdateInfo(zeno::CustomUI& customui, zeno::ParamsUpdateInfo& editUpdateInfo);
    QStandardItemModel* toStandardModel() const;

private:
    QVector<QPersistentModelIndex> m_items;    //一个group下所有的param
    ParamsModel* m_paramsModel;

    bool m_bIscloned;
    QVector<ParamItem> m_clonedItems;
};

class objParamOutputModel : public QAbstractListModel
{
    Q_OBJECT
        typedef QAbstractListModel _base;

    struct _ParamItem {
        QPersistentModelIndex m_index;  //指向ParamsModel真正储存的参数
    };

public:
    objParamOutputModel(zeno::ObjectParams params, CustomUIModel* pModel);
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE bool insertRow(int row, QString name);
    Q_INVOKABLE bool removeRow(int row);

    void reset();
    void exportCustomuiAndEdittedUpdateInfo(zeno::CustomUI& customui, zeno::ParamsUpdateInfo& editUpdateInfo);
    QStandardItemModel* toStandardModel() const;

private:
    QVector<QPersistentModelIndex> m_items;    //一个group下所有的param
    ParamsModel* m_paramsModel;

    bool m_bIscloned;
    QVector<ParamItem> m_clonedItems;
};

#endif
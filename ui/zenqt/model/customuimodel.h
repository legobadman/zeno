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
class ParamOutputModel;


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
    CustomUIModel(ParamsModel* params, QObject* parent = nullptr);
    Q_INVOKABLE ParamTabModel* tabModel() const;
    Q_INVOKABLE ParamOutputModel* outputModel() const;
    Q_INVOKABLE ParamsModel* coreModel() const;
    void initCustomuiConnections(QStandardItemModel* customuiStandarditemModel);

private:
    ParamsModel* m_params;
    ParamTabModel* m_tabModel;
    ParamOutputModel* m_outputModel;
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

    bool insertRow(int row, QString& name);
    bool removeRow(int row);

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

    bool insertRow(int row, QString& name);
    bool removeRow(int row);

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
    Q_INVOKABLE ParamGroupModel* groupModel() { return m_groupModel; }

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    QHash<int, QByteArray> roleNames() const override;

    bool insertRow(int row, QString& name);
    bool removeRow(int row);

private:
    QVector<QPersistentModelIndex> m_items;    //一个group下所有的param
    ParamGroupModel* m_groupModel;
};


class ParamOutputModel : public QAbstractListModel
{
    Q_OBJECT
    typedef QAbstractListModel _base;

    struct _ParamItem {
        QPersistentModelIndex m_index;  //指向ParamsModel真正储存的参数
    };

public:
    ParamOutputModel(zeno::PrimitiveParams params, CustomUIModel* pModel);
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    QHash<int, QByteArray> roleNames() const override;

private:
    QVector<QPersistentModelIndex> m_items;    //一个group下所有的param
};

#endif
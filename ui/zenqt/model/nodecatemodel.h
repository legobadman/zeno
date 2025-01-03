#pragma once

#ifndef __NODE_CATE_MODEL_H__
#define __NODE_CATE_MODEL_H__

#include <QObject>
#include <QAbstractListModel>
#include <QString>
#include <QQuickItem>

class NodeCateModel : public QAbstractListModel
{
    Q_OBJECT
    typedef QAbstractListModel _base;
    QML_ELEMENT

    struct _CateItem
    {
        QString cate;
        QStringList nodes;
    };

public:
    NodeCateModel(QObject* parent = nullptr);
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    QHash<int, QByteArray> roleNames() const override;

private:
    QVector<_CateItem> m_cates;
};

#endif
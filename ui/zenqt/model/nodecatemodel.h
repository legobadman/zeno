#pragma once

#ifndef __NODE_CATE_MODEL_H__
#define __NODE_CATE_MODEL_H__

#include <QObject>
#include <QAbstractListModel>
#include <QString>
#include <QQuickItem>

class MenuEventFilter : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit MenuEventFilter(QObject* parent = nullptr) : QObject(parent) {}

    Q_INVOKABLE void listenTo(QObject* object)
    {
        if (!object)
            return;

        QObjectList children = object->children();
        for (auto pObject : children) {
            pObject->installEventFilter(this);
        }
    }

signals:
    void textAppended(QString);
    void textRemoved();

protected:
    bool eventFilter(QObject* obj, QEvent* event) override {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
            int key = keyEvent->key();
            QChar c(key);
            if (c.isLetterOrNumber())
            {
                emit textAppended(c);
                return true;
            }
            if (c == Qt::Key_Backspace)
            {
                emit textRemoved();
                return true;
            }
        }
        return QObject::eventFilter(obj, event);
    }
};

class NodeCateModel : public QAbstractListModel
{
    Q_OBJECT
    typedef QAbstractListModel _base;
    QML_ELEMENT

    struct _CateItem
    {
        QString cate;
        QStringList nodes;
        bool iscate = true;
    };

public:
    NodeCateModel(QObject* parent = nullptr);
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void search(const QString& name);
    Q_INVOKABLE bool iscatepage() const;
    bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;
    void clear();

private:
    QVector<_CateItem> m_cates;
    QVector<_CateItem> m_cache_cates;
    QString m_search;
};

#endif
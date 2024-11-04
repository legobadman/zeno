#pragma once

#ifndef __GEOMETRY_MODEL_H__
#define __GEOMETRY_MODEL_H__

#include <QObject>
#include <QAbstractTableModel>
#include <map>
#include <vector>
#include <zeno/types/GeometryObject.h>


struct VertexInfo {
    int face;
    int point;
};

enum AttrColType {
    AttrInt,
    AttrFloat,
    AttrString,
};

struct AttributeInfo {
    std::string name;
    AttrColType type;
    char channel = 0;
};

class VertexModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    VertexModel(zeno::GeometryObject* pObject, QObject* parent = nullptr);
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent) const override;
    bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;

private:
    //std::vector<VertexInfo> m_vertices;
    std::map<int, AttributeInfo> m_colMap;
    zeno::GeometryObject* m_object;
};

class PointModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    PointModel(zeno::GeometryObject* pObject, QObject* parent = nullptr);
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent) const override;
    bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;

private:
    std::map<int, AttributeInfo> m_colMap;
    zeno::GeometryObject* m_object;
};

class FaceModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    FaceModel(zeno::GeometryObject* pObject, QObject* parent = nullptr);
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent) const override;
    bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;

private:
    std::map<int, AttributeInfo> m_colMap;
    zeno::GeometryObject* m_object;
};

#endif
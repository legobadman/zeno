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
    std::string showName;
    AttrColType type;
    char channel = 0;
};

class VertexModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    VertexModel(std::shared_ptr<zeno::GeometryObject> pObject, QObject* parent = nullptr);
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent) const override;
    bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void setGeoObject(std::shared_ptr<zeno::GeometryObject> spObject);

private:
    QMap<int, AttributeInfo> m_colMap;
    std::weak_ptr<zeno::GeometryObject> m_object;
    int m_nvertices;
};

class PointModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    PointModel(std::shared_ptr<zeno::GeometryObject> pObject, QObject* parent = nullptr);
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent) const override;
    bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void setGeoObject(std::shared_ptr<zeno::GeometryObject> pObject);

private:
    QMap<int, AttributeInfo> m_colMap;
    std::weak_ptr<zeno::GeometryObject> m_object;
    int m_npoints;
};

class FaceModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    FaceModel(std::shared_ptr<zeno::GeometryObject> pObject, QObject* parent = nullptr);
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent) const override;
    bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void setGeoObject(std::shared_ptr<zeno::GeometryObject> pObject);

private:
    QMap<int, AttributeInfo> m_colMap;
    std::weak_ptr<zeno::GeometryObject> m_object;

    int m_nfaces;
};

class GeomDetailModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    GeomDetailModel(std::shared_ptr<zeno::GeometryObject> pObject, QObject* parent = nullptr);
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override { return 1; }
    int columnCount(const QModelIndex& parent) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    void setGeoObject(std::shared_ptr<zeno::GeometryObject> pObject);

private:
    QMap<int, AttributeInfo> m_colMap;
    std::weak_ptr<zeno::GeometryObject> m_object;
};

#endif
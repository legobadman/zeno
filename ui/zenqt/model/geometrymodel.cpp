#include "geometrymodel.h"


std::map<int, AttributeInfo> initColMapping(zeno::GeometryObject* pObject, zeno::GeoAttrGroup group)
{
    std::map<int, AttributeInfo> colMapping;
    std::vector<std::string> names = pObject->get_attr_names(group);
    int nCol = 0;
    if (zeno::ATTR_VERTEX == group) {
        colMapping.insert(std::make_pair(nCol++, AttributeInfo{ "Point Number", AttrInt }));
    }

    for (auto name : names) {
        //TODO: pos这种要放到最前头
        zeno::GeoAttrType type = pObject->get_attr_type(group, name);
        if (type == zeno::ATTR_VEC2 || type == zeno::ATTR_VEC3 || type == zeno::ATTR_VEC4) {
            colMapping.insert(std::make_pair(nCol++, AttributeInfo{ name + ".x", AttrFloat, 0 }));
            colMapping.insert(std::make_pair(nCol++, AttributeInfo{ name + ".y", AttrFloat, 1 }));
            if (type == zeno::ATTR_VEC3 || type == zeno::ATTR_VEC4) {
                colMapping.insert(std::make_pair(nCol++, AttributeInfo{ name + ".z", AttrFloat, 2 }));
                if (type == zeno::ATTR_VEC4) {
                    colMapping.insert(std::make_pair(nCol++, AttributeInfo{ name + ".w", AttrFloat, 3 }));
                }
            }
        }
        else {
            if (type == zeno::ATTR_FLOAT)
                colMapping.insert(std::make_pair(nCol++, AttributeInfo{ name, AttrFloat }));
            else if (type == zeno::ATTR_INT)
                colMapping.insert(std::make_pair(nCol++, AttributeInfo{ name, AttrInt }));
            else if (type == zeno::ATTR_STRING)
                colMapping.insert(std::make_pair(nCol++, AttributeInfo{ name, AttrString }));
        }
    }
    return colMapping;
}



/// <summary>
/// 
/// </summary>
/// <param name="pObject"></param>
VertexModel::VertexModel(zeno::GeometryObject* pObject, QObject* parent) : m_object(pObject)
{
    m_colMap = initColMapping(m_object, zeno::ATTR_VERTEX);
}

QVariant VertexModel::data(const QModelIndex& index, int role) const {
    int row = index.row(), col = index.column();
    if (role == Qt::DisplayRole) {
        if (col == 0) {
            //Point Number
            auto& [face, point] = m_object->vertex_info(row);
            return QString("%1:%2").arg(face).arg(point);
        }
        else {
            AttributeInfo info = m_colMap.find(col)->second;
            if (info.type == AttrFloat) {
                float val = m_object->get_elem<float>(zeno::ATTR_VERTEX, info.name, info.channel, row);
                return QString::number(val);
            }
            else if (info.type == AttrInt) {
                int val = m_object->get_elem<int>(zeno::ATTR_VERTEX, info.name, info.channel, row);
                return QString::number(val);
            }
            else if (info.type == AttrString) {
                std::string val = m_object->get_elem<std::string>(zeno::ATTR_VERTEX, info.name, info.channel, row);
                return QString::fromStdString(val);
            }
        }
    }
    return QVariant();
}

int VertexModel::rowCount(const QModelIndex& parent) const {
    return m_object->nvertices();
}

int VertexModel::columnCount(const QModelIndex& parent) const {
    return m_colMap.size();
}

bool VertexModel::removeRows(int row, int count, const QModelIndex& parent) {
    return QAbstractTableModel::removeRows(row, count, parent);
}

/// <summary>
///
/// </summary>
/// <param name="pObject"></param>
PointModel::PointModel(zeno::GeometryObject* pObject, QObject* parent) : m_object(pObject) {
    m_colMap = initColMapping(pObject, zeno::ATTR_POINT);
}

QVariant PointModel::data(const QModelIndex& index, int role) const {
    int row = index.row(), col = index.column();
    if (role == Qt::DisplayRole) {
        AttributeInfo info = m_colMap.find(col)->second;
        if (info.type == AttrFloat) {
            float val = m_object->get_elem<float>(zeno::ATTR_POINT, info.name, info.channel, row);
            return QString::number(val);
        }
        else if (info.type == AttrInt) {
            int val = m_object->get_elem<int>(zeno::ATTR_POINT, info.name, info.channel, row);
            return QString::number(val);
        }
        else if (info.type == AttrString) {
            std::string val = m_object->get_elem<std::string>(zeno::ATTR_POINT, info.name, info.channel, row);
            return QString::fromStdString(val);
        }
    }
    return QVariant();
}

bool PointModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    return false;
}

int PointModel::rowCount(const QModelIndex& parent) const {
    return m_object->npoints();
}

int PointModel::columnCount(const QModelIndex& parent) const {
    return m_colMap.size();
}

bool PointModel::removeRows(int row, int count, const QModelIndex& parent) {
    return QAbstractTableModel::removeRows(row, count, parent);
}

/// <summary>
/// </summary>
/// <param name="pObject"></param>

FaceModel::FaceModel(zeno::GeometryObject* pObject, QObject* parent) : m_object(pObject) {
    m_colMap = initColMapping(m_object, zeno::ATTR_FACE);
}

QVariant FaceModel::data(const QModelIndex& index, int role) const {
    int row = index.row(), col = index.column();
    if (role == Qt::DisplayRole) {
        AttributeInfo info = m_colMap.find(col)->second;
        if (info.type == AttrFloat) {
            float val = m_object->get_elem<float>(zeno::ATTR_FACE, info.name, info.channel, row);
            return QString::number(val);
        }
        else if (info.type == AttrInt) {
            int val = m_object->get_elem<int>(zeno::ATTR_FACE, info.name, info.channel, row);
            return QString::number(val);
        }
        else if (info.type == AttrString) {
            std::string val = m_object->get_elem<std::string>(zeno::ATTR_FACE, info.name, info.channel, row);
            return QString::fromStdString(val);
        }
    }
    return QVariant();
}

int FaceModel::rowCount(const QModelIndex& parent) const {
    return m_object->nfaces();
}

int FaceModel::columnCount(const QModelIndex& parent) const {
    return m_colMap.size();
}

bool FaceModel::removeRows(int row, int count, const QModelIndex& parent) {
    return QAbstractTableModel::removeRows(row, count, parent);
}

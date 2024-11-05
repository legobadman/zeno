#include "geometrymodel.h"


QMap<int, AttributeInfo> initColMapping(zeno::GeometryObject* pObject, zeno::GeoAttrGroup group)
{
    QMap<int, AttributeInfo> colMapping;
    std::vector<std::string> names = pObject->get_attr_names(group);
    int nCol = 0;
    if (zeno::ATTR_VERTEX == group) {
        colMapping[nCol++] = AttributeInfo{ "Face-Vertex", "Face-Vertex", AttrInt };
        colMapping[nCol++] = AttributeInfo{ "Point Number", "Point Number", AttrInt };
    }

    for (auto name : names) {
        QString qName = QString::fromStdString(name);
        //TODO: pos这种要放到最前头
        zeno::GeoAttrType type = pObject->get_attr_type(group, name);
        if (type == zeno::ATTR_VEC2 || type == zeno::ATTR_VEC3 || type == zeno::ATTR_VEC4) {
            colMapping[nCol++] = AttributeInfo{ name, name + ".x", AttrFloat, 'x'};
            colMapping[nCol++] = AttributeInfo{ name, name + ".y", AttrFloat, 'y'};
            if (type == zeno::ATTR_VEC3 || type == zeno::ATTR_VEC4) {
                colMapping[nCol++] = AttributeInfo{ name, name + ".z", AttrFloat, 'z'};
                if (type == zeno::ATTR_VEC4) {
                    colMapping[nCol++] = AttributeInfo{ name, name + ".w", AttrFloat, 'w'};
                }
            }
        }
        else {
            if (type == zeno::ATTR_FLOAT)
                colMapping[nCol++] = AttributeInfo{ name, name, AttrFloat };
            else if (type == zeno::ATTR_INT)
                colMapping[nCol++] = AttributeInfo{ name, name, AttrInt };
            else if (type == zeno::ATTR_STRING)
                colMapping[nCol++] = AttributeInfo{ name, name, AttrString };
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
        if (col == 0 || col == 1) {
            //Point Number
            auto& [face, vertidx, point] = m_object->vertex_info(row);
            if (col == 0)
                return QString("%1:%2").arg(face).arg(vertidx);
            else
                return QString::number(point);
        }
        else {
            const AttributeInfo& info = m_colMap[col];
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

QVariant VertexModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            return QString::fromStdString(m_colMap[section].showName);
        }
        else {
            return QString::number(section);
        }
    }
    return QAbstractTableModel::headerData(section, orientation, role);
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
        const AttributeInfo& info = m_colMap[col];
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

QVariant PointModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            const AttributeInfo& info = m_colMap[section];
            return QString::fromStdString(info.showName);
        }
        else {
            return QString::number(section);
        }
    }
    return QAbstractTableModel::headerData(section, orientation, role);
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
        const AttributeInfo& info = m_colMap[col];
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

#include "geometrymodel.h"

void addCol(std::shared_ptr<zeno::GeometryObject> pObject, zeno::GeoAttrGroup group, QMap<int, AttributeInfo>& colMapping, std::string name, int& nCol) {
    QString qName = QString::fromStdString(name);
    //TODO: pos这种要放到最前头
    zeno::GeoAttrType type = pObject->get_attr_type(group, name);
    if (type == zeno::ATTR_VEC2 || type == zeno::ATTR_VEC3 || type == zeno::ATTR_VEC4) {
        colMapping[nCol++] = AttributeInfo{ name, name + ".x", AttrFloat, 'x' };
        colMapping[nCol++] = AttributeInfo{ name, name + ".y", AttrFloat, 'y' };
        if (type == zeno::ATTR_VEC3 || type == zeno::ATTR_VEC4) {
            colMapping[nCol++] = AttributeInfo{ name, name + ".z", AttrFloat, 'z' };
            if (type == zeno::ATTR_VEC4) {
                colMapping[nCol++] = AttributeInfo{ name, name + ".w", AttrFloat, 'w' };
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

QMap<int, AttributeInfo> initColMapping(std::shared_ptr<zeno::GeometryObject> pObject, zeno::GeoAttrGroup group)
{
    QMap<int, AttributeInfo> colMapping;
    std::vector<std::string> names = pObject->get_attr_names(group);
    int nCol = 0;
    if (zeno::ATTR_VERTEX == group) {
        colMapping[nCol++] = AttributeInfo{ "Face-Vertex", "Face-Vertex", AttrInt };
        colMapping[nCol++] = AttributeInfo{ "Point Number", "Point Number", AttrInt };
    }

    for (auto name : names) {
        addCol(pObject, group, colMapping, name, nCol);
    }
    return colMapping;
}



/// <summary>
/// 
/// </summary>
/// <param name="pObject"></param>
VertexModel::VertexModel(std::shared_ptr<zeno::GeometryObject> spObject, QObject* parent) : m_object(spObject), m_nvertices(spObject->nvertices())
{
    m_colMap = initColMapping(spObject, zeno::ATTR_VERTEX);
    spObject->register_add_vertex([this](int vertext_linearIdx) {
        if (vertext_linearIdx != -1) {
            beginInsertRows(QModelIndex(), vertext_linearIdx, vertext_linearIdx);
            m_nvertices++;
            endInsertRows();
        }
    });
    spObject->register_remove_vertex([this](int vertext_linearIdx) {
        if (vertext_linearIdx != -1) {
            beginRemoveRows(QModelIndex(), vertext_linearIdx, vertext_linearIdx);
            m_nvertices--;
            endRemoveRows();
        }
    });
    spObject->register_reset_vertices([&]() {
        beginResetModel();
        m_nvertices = spObject->nvertices();
        endResetModel();
    });
    //m_object->register_create_vertex_attr([this](std::string attrname) {
    //    beginInsertColumns(QModelIndex(), m_colMap.size(), m_colMap.size());
    //    addCol(m_object, zeno::ATTR_VERTEX, m_colMap, attrname, m_colMap.size());
    //    endInsertColumns();
    //});
    //m_object->register_delete_vertex_attr([this](std::string attrname) {
    //    int rmidx = -1;
    //    for (QMap<int, AttributeInfo>::const_iterator it = m_colMap.begin(); it != m_colMap.end(); ++it) {
    //        if (it.value().name == attrname) {
    //            rmidx = it.key();
    //            break;
    //        }
    //    }
    //    if (rmidx != -1) {
    //        beginRemoveColumns(QModelIndex(), rmidx, rmidx);
    //        m_colMap.remove(rmidx);
    //        QMap<int, AttributeInfo> newColmap;
    //        for (auto& it = m_colMap.begin(); it != m_colMap.end(); ++it) {
    //            if (it.key() > rmidx) {
    //                newColmap.insert(it.key() - 1, it.value());
    //            } else {
    //                newColmap.insert(it.key(), it.value());
    //            }
    //        }
    //        m_colMap.swap(newColmap);
    //        endRemoveColumns();
    //    }
    //});
}

QVariant VertexModel::data(const QModelIndex& index, int role) const {
    auto spObject = m_object.lock();
    if (!spObject) {
        return QVariant();
    }
    int row = index.row(), col = index.column();
    if (role == Qt::DisplayRole) {
        if (col == 0 || col == 1) {
            //Point Number
            auto& [face, vertidx, point] = spObject->vertex_info(row);
            if (col == 0)
                return QString("%1:%2").arg(face).arg(vertidx);
            else
                return QString::number(point);
        }
        else {
            const AttributeInfo& info = m_colMap[col];
            if (info.type == AttrFloat) {
                float val = spObject->get_elem<float>(zeno::ATTR_VERTEX, info.name, info.channel, row);
                return QString::number(val);
            }
            else if (info.type == AttrInt) {
                int val = spObject->get_elem<int>(zeno::ATTR_VERTEX, info.name, info.channel, row);
                return QString::number(val);
            }
            else if (info.type == AttrString) {
                std::string val = spObject->get_elem<std::string>(zeno::ATTR_VERTEX, info.name, info.channel, row);
                return QString::fromStdString(val);
            }
        }
    }
    return QVariant();
}

int VertexModel::rowCount(const QModelIndex& parent) const {
    return m_nvertices;
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

void VertexModel::setGeoObject(std::shared_ptr<zeno::GeometryObject> spObject)
{
    beginResetModel();
    m_object = spObject;
    m_nvertices = spObject->nvertices();
    m_colMap = initColMapping(spObject, zeno::ATTR_VERTEX);
    endResetModel();
}

bool VertexModel::removeRows(int row, int count, const QModelIndex& parent) {
    return QAbstractTableModel::removeRows(row, count, parent);
}

/// <summary>
///
/// </summary>
/// <param name="pObject"></param>
PointModel::PointModel(std::shared_ptr<zeno::GeometryObject> spObject, QObject* parent) : m_object(spObject), m_npoints(spObject->npoints()) {
    m_colMap = initColMapping(spObject, zeno::ATTR_POINT);
    spObject->register_add_point([this](int ptnum) {
        if (ptnum != -1) {
            beginInsertRows(QModelIndex(), ptnum, ptnum);
            m_npoints++;
            endInsertRows();
        }
    });
    spObject->register_remove_point([this](int ptnum) {
        if (ptnum != -1) {
            beginRemoveRows(QModelIndex(), ptnum, ptnum);
            m_npoints--;
            endRemoveRows();
        }
    });
    //m_object->register_create_point_attr([this](std::string attrname) {
    //    beginInsertColumns(QModelIndex(), m_colMap.size(), m_colMap.size());
    //    addCol(m_object, zeno::ATTR_POINT, m_colMap, attrname, m_colMap.size());
    //    endInsertColumns();
    //});
    //m_object->register_delete_point_attr([this](std::string attrname) {
    //    int rmidx = -1;
    //    for (QMap<int, AttributeInfo>::const_iterator it = m_colMap.begin(); it != m_colMap.end(); ++it) {
    //        if (it.value().name == attrname) {
    //            rmidx = it.key();
    //            break;
    //        }
    //    }
    //    if (rmidx != -1) {
    //        beginRemoveColumns(QModelIndex(), rmidx, rmidx);
    //        m_colMap.remove(rmidx);
    //        QMap<int, AttributeInfo> newColmap;
    //        for (auto& it = m_colMap.begin(); it != m_colMap.end(); ++it) {
    //            if (it.key() > rmidx) {
    //                newColmap.insert(it.key() - 1, it.value());
    //            }
    //            else {
    //                newColmap.insert(it.key(), it.value());
    //            }
    //        }
    //        m_colMap.swap(newColmap);
    //        endRemoveColumns();
    //    }
    //});
}

QVariant PointModel::data(const QModelIndex& index, int role) const {
    auto spObject = m_object.lock();
    if (!spObject)
        return QVariant();
    int row = index.row(), col = index.column();
    if (role == Qt::DisplayRole) {
        const AttributeInfo& info = m_colMap[col];
        if (info.type == AttrFloat) {
            float val = spObject->get_elem<float>(zeno::ATTR_POINT, info.name, info.channel, row);
            return QString::number(val);
        }
        else if (info.type == AttrInt) {
            int val = spObject->get_elem<int>(zeno::ATTR_POINT, info.name, info.channel, row);
            return QString::number(val);
        }
        else if (info.type == AttrString) {
            std::string val = spObject->get_elem<std::string>(zeno::ATTR_POINT, info.name, info.channel, row);
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

void PointModel::setGeoObject(std::shared_ptr<zeno::GeometryObject> spObject)
{
    beginResetModel();
    m_object = spObject;
    m_npoints = spObject->npoints();
    m_colMap = initColMapping(spObject, zeno::ATTR_POINT);
    endResetModel();
}

int PointModel::rowCount(const QModelIndex& parent) const {
    return m_npoints;
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

FaceModel::FaceModel(std::shared_ptr<zeno::GeometryObject> spObject, QObject* parent) : m_object(spObject), m_nfaces(spObject->nfaces()) {
    m_colMap = initColMapping(spObject, zeno::ATTR_FACE);
    spObject->register_add_face([this](int faceid) {
        if (faceid != -1) {
            beginInsertRows(QModelIndex(), faceid, faceid);
            m_nfaces++;
            endInsertRows();
        }
    });
    spObject->register_remove_face([this](int faceid) {
        if (faceid != -1) {
            beginRemoveRows(QModelIndex(), faceid, faceid);
            m_nfaces--;
            endRemoveRows();
        }
    });
    spObject->register_reset_faces([this]() {
        beginResetModel();
        auto _spObject = m_object.lock();
        m_nfaces = _spObject->nfaces();
        endResetModel();
    });
    //m_object->register_create_face_attr([this](std::string attrname) {
    //    beginInsertColumns(QModelIndex(), m_colMap.size(), m_colMap.size());
    //    addCol(m_object, zeno::ATTR_FACE, m_colMap, attrname, m_colMap.size());
    //    endInsertColumns();
    //});
    //m_object->register_delete_face_attr([this](std::string attrname) {
    //    int rmidx = -1;
    //    for (QMap<int, AttributeInfo>::const_iterator it = m_colMap.begin(); it != m_colMap.end(); ++it) {
    //        if (it.value().name == attrname) {
    //            rmidx = it.key();
    //            break;
    //        }
    //    }
    //    if (rmidx != -1) {
    //        beginRemoveColumns(QModelIndex(), rmidx, rmidx);
    //        m_colMap.remove(rmidx);
    //        QMap<int, AttributeInfo> newColmap;
    //        for (auto& it = m_colMap.begin(); it != m_colMap.end(); ++it) {
    //            if (it.key() > rmidx) {
    //                newColmap.insert(it.key() - 1, it.value());
    //            }
    //            else {
    //                newColmap.insert(it.key(), it.value());
    //            }
    //        }
    //        m_colMap.swap(newColmap);
    //        endRemoveColumns();
    //    }
    //});
}

QVariant FaceModel::data(const QModelIndex& index, int role) const {
    auto spObject = m_object.lock();
    if (!spObject) {
        return QVariant();
    }
    int row = index.row(), col = index.column();
    if (role == Qt::DisplayRole) {
        const AttributeInfo& info = m_colMap[col];
        if (info.type == AttrFloat) {
            float val = spObject->get_elem<float>(zeno::ATTR_FACE, info.name, info.channel, row);
            return QString::number(val);
        }
        else if (info.type == AttrInt) {
            int val = spObject->get_elem<int>(zeno::ATTR_FACE, info.name, info.channel, row);
            return QString::number(val);
        }
        else if (info.type == AttrString) {
            std::string val = spObject->get_elem<std::string>(zeno::ATTR_FACE, info.name, info.channel, row);
            return QString::fromStdString(val);
        }
    }
    return QVariant();
}

int FaceModel::rowCount(const QModelIndex& parent) const {
    return m_nfaces;
}

int FaceModel::columnCount(const QModelIndex& parent) const {
    return m_colMap.size();
}

bool FaceModel::removeRows(int row, int count, const QModelIndex& parent) {
    return QAbstractTableModel::removeRows(row, count, parent);
}

QVariant FaceModel::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/) const
{
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

void FaceModel::setGeoObject(std::shared_ptr<zeno::GeometryObject> spObject)
{
    if (!spObject) {
        return;
    }
    beginResetModel();
    m_object = spObject;
    m_nfaces = spObject->nfaces();
    m_colMap = initColMapping(spObject, zeno::ATTR_FACE);
    endResetModel();
}


GeomDetailModel::GeomDetailModel(std::shared_ptr<zeno::GeometryObject> spObject, QObject* parent)
    : QAbstractTableModel(parent)
    , m_object(spObject)
{
    m_colMap = initColMapping(spObject, zeno::ATTR_GEO);
}

QVariant GeomDetailModel::data(const QModelIndex& index, int role) const
{
    auto spObject = m_object.lock();
    if (!spObject)
        return QVariant();

    int row = index.row(), col = index.column();
    if (role == Qt::DisplayRole) {
        const AttributeInfo& info = m_colMap[col];
        if (info.type == AttrFloat) {
            float val = spObject->get_elem<float>(zeno::ATTR_GEO, info.name, info.channel, row);
            return QString::number(val);
        }
        else if (info.type == AttrInt) {
            int val = spObject->get_elem<int>(zeno::ATTR_GEO, info.name, info.channel, row);
            return QString::number(val);
        }
        else if (info.type == AttrString) {
            std::string val = spObject->get_elem<std::string>(zeno::ATTR_GEO, info.name, info.channel, row);
            return QString::fromStdString(val);
        }
    }
    return QVariant();
}

int GeomDetailModel::columnCount(const QModelIndex& parent) const {
    return m_colMap.size();
}

QVariant GeomDetailModel::headerData(int section, Qt::Orientation orientation, int role) const {
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

void GeomDetailModel::setGeoObject(std::shared_ptr<zeno::GeometryObject> spObject) {
    beginResetModel();
    m_object = spObject;
    m_colMap = initColMapping(spObject, zeno::ATTR_GEO);
    endResetModel();
}

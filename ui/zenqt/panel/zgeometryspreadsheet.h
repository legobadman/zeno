#pragma once

#ifndef __ZGEOMETRY_SPREADSHEET_H__
#define __ZGEOMETRY_SPREADSHEET_H__

#include <QtWidgets>
#include <zeno/types/IGeometryObject.h>

class ZToolBarButton;
class GraphModel;

class ZGeometrySpreadsheet : public QWidget
{
    Q_OBJECT
public:
    ZGeometrySpreadsheet(QWidget* parent = nullptr);
    void setGeometry(GraphModel* subgraph, QModelIndex nodeidx, std::shared_ptr<zeno::GeometryObject_Adapter> spObject);

public slots:
    void onNodeRemoved(QString nodename);
    void onNodeDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles);

private:
    void clearModel();

    QLabel* m_lblNode;
    ZToolBarButton* m_vertex;
    ZToolBarButton* m_point;
    ZToolBarButton* m_face;
    ZToolBarButton* m_geom;
    QStackedWidget* m_views;

    GraphModel* m_model;
    QPersistentModelIndex m_nodeIdx;
};


#endif
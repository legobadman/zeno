#pragma once

#ifndef __ZGEOMETRY_SPREADSHEET_H__
#define __ZGEOMETRY_SPREADSHEET_H__

#include <QtWidgets>

class ZToolBarButton;

namespace zeno {
    class GeometryObject;
}

class ZGeometrySpreadsheet : public QWidget
{
    Q_OBJECT
public:
    ZGeometrySpreadsheet(QWidget* parent = nullptr);
    void setGeometry(zeno::GeometryObject* pObject);

private:
    QLabel* m_lblNode;
    ZToolBarButton* m_vertex;
    ZToolBarButton* m_point;
    ZToolBarButton* m_face;
    ZToolBarButton* m_geom;
    QStackedWidget* m_views;
};


#endif
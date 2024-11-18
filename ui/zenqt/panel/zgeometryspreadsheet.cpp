#include "zgeometryspreadsheet.h"
#include "../layout/docktabcontent.h"
#include <zeno/types/GeometryObject.h>
#include "model/geometrymodel.h"
#include "zenoapplication.h"
#include "model/graphsmanager.h"


ZGeometrySpreadsheet::ZGeometrySpreadsheet(QWidget* parent)
    : QWidget(parent)
    , m_lblNode(new QLabel)
    , m_views(new QStackedWidget(this))
    , m_vertex(new ZToolBarButton(true, ":/icons/fixpanel.svg", ":/icons/fixpanel-on.svg"))
    , m_point(new ZToolBarButton(true, ":/icons/wiki.svg", ":/icons/wiki-on.svg"))
    , m_face(new ZToolBarButton(true, ":/icons/settings.svg", ":/icons/settings-on.svg"))
    , m_geom(new ZToolBarButton(true, ":/icons/toolbar_search_idle.svg", ":/icons/toolbar_search_light.svg"))
{
    m_views->addWidget(new QTableView); //vertex
    m_views->addWidget(new QTableView); //point
    m_views->addWidget(new QTableView); //face
    m_views->addWidget(new QTableView); //geom

    QHBoxLayout* pToolbarLayout = new QHBoxLayout;
    pToolbarLayout->addWidget(m_lblNode);
    pToolbarLayout->addWidget(m_vertex);
    pToolbarLayout->addWidget(m_point);
    pToolbarLayout->addWidget(m_face);
    pToolbarLayout->addWidget(m_geom);

    QVBoxLayout* pMainLayout = new QVBoxLayout;
    pMainLayout->addLayout(pToolbarLayout);
    pMainLayout->addWidget(m_views);

    m_vertex->setChecked(true);
    m_point->setChecked(false);
    m_face->setChecked(false);
    m_geom->setChecked(false);

    connect(m_vertex, &ZToolBarButton::toggled, [&](bool bChecked) {
        m_point->setChecked(!bChecked);
        m_face->setChecked(!bChecked);
        m_geom->setChecked(!bChecked);
        m_views->setCurrentIndex(0);
        });

    connect(m_point, &ZToolBarButton::toggled, [&](bool bChecked) {
        m_vertex->setChecked(!bChecked);
        m_face->setChecked(!bChecked);
        m_geom->setChecked(!bChecked);
        m_views->setCurrentIndex(1);
        });

    connect(m_face, &ZToolBarButton::toggled, [&](bool bChecked) {
        m_vertex->setChecked(!bChecked);
        m_point->setChecked(!bChecked);
        m_geom->setChecked(!bChecked);
        m_views->setCurrentIndex(2);
        });

    connect(m_geom, &ZToolBarButton::toggled, [&](bool bChecked) {
        m_vertex->setChecked(!bChecked);
        m_face->setChecked(!bChecked);
        m_point->setChecked(!bChecked);
        m_views->setCurrentIndex(3);
        });

    connect(zenoApp->graphsManager(), &GraphsManager::fileClosed, this, [this]() {
        for (int i = 0; i < m_views->count(); ++i) {
            QTableView* view = qobject_cast<QTableView*>(m_views->widget(i));
            if (QAbstractItemModel* model = view->model()) {
                view->setModel(nullptr);
                delete model;
            }
        }
    });

    setLayout(pMainLayout);
}

void ZGeometrySpreadsheet::setGeometry(zeno::GeometryObject* pObject) {
    QTableView* view = qobject_cast<QTableView*>(m_views->widget(0));
    view->setModel(new VertexModel(pObject));

    view = qobject_cast<QTableView*>(m_views->widget(1));
    view->setModel(new PointModel(pObject));

    view = qobject_cast<QTableView*>(m_views->widget(2));
    view->setModel(new FaceModel(pObject));

    //TODO: geom model
}
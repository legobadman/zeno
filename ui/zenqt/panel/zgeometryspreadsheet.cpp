#include "zgeometryspreadsheet.h"
#include "../layout/docktabcontent.h"
#include <zeno/types/GeometryObject.h>
#include "model/geometrymodel.h"
#include "zenoapplication.h"
#include "model/graphsmanager.h"
#include "declmetatype.h"


ZGeometrySpreadsheet::ZGeometrySpreadsheet(QWidget* parent)
    : QWidget(parent)
    , m_lblNode(new QLabel)
    , m_views(new QStackedWidget(this))
    , m_vertex(new ZToolBarButton(true, ":/icons/fixpanel.svg", ":/icons/fixpanel-on.svg"))
    , m_point(new ZToolBarButton(true, ":/icons/wiki.svg", ":/icons/wiki-on.svg"))
    , m_face(new ZToolBarButton(true, ":/icons/settings.svg", ":/icons/settings-on.svg"))
    , m_geom(new ZToolBarButton(true, ":/icons/toolbar_search_idle.svg", ":/icons/toolbar_search_light.svg"))
    , m_model(nullptr)
    , m_nodeIdx(QModelIndex())
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
        clearModel();
    });

    setLayout(pMainLayout);
}

void ZGeometrySpreadsheet::setGeometry(GraphModel* subgraph, QModelIndex nodeidx, zeno::GeometryObject* pObject) {
    if (subgraph) {
        m_model = subgraph;
        connect(m_model, &GraphModel::nodeRemoved, this, &ZGeometrySpreadsheet::onNodeRemoved, Qt::UniqueConnection);
        connect(m_model, &GraphModel::dataChanged, this, &ZGeometrySpreadsheet::onNodeDataChanged, Qt::UniqueConnection);
    }
    if (nodeidx.isValid()) {
        m_nodeIdx = nodeidx;
    }

    QTableView* view = qobject_cast<QTableView*>(m_views->widget(0));
    if (VertexModel* model = qobject_cast<VertexModel*>(view->model())) {
        model->setGeoObject(pObject);
    } else {
        view->setModel(new VertexModel(pObject));
    }

    view = qobject_cast<QTableView*>(m_views->widget(1));
    if (PointModel* model = qobject_cast<PointModel*>(view->model())) {
        model->setGeoObject(pObject);
    }
    else {
        view->setModel(new PointModel(pObject));
    }

    view = qobject_cast<QTableView*>(m_views->widget(2));
    if (FaceModel* model = qobject_cast<FaceModel*>(view->model())) {
        model->setGeoObject(pObject);
    }
    else {
        view->setModel(new FaceModel(pObject));
    }

    mmm = pObject;
    //TODO: geom model
}

void ZGeometrySpreadsheet::onNodeRemoved(QString nodename)
{
    if (!m_nodeIdx.isValid()) {
        clearModel();
    }
}

void ZGeometrySpreadsheet::onNodeDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles)
{
    if (!roles.empty() && roles[0] == QtRole::ROLE_NODE_RUN_STATE) {
        zeno::NodeRunStatus currStatus = topLeft.data(QtRole::ROLE_NODE_RUN_STATE).value<NodeState>().runstatus;
        if (currStatus == zeno::Node_Running) {
            clearModel();
        }
    }
}

void ZGeometrySpreadsheet::clearModel()
{
    for (int i = 0; i < m_views->count(); ++i) {
        QTableView* view = qobject_cast<QTableView*>(m_views->widget(i));
        if (QAbstractItemModel* model = view->model()) {
            view->setModel(nullptr);
            delete model;
        }
    }
}

#include "zgeometryspreadsheet.h"
#include "../layout/docktabcontent.h"
#include <zeno/types/IGeometryObject.h>
#include "model/geometrymodel.h"
#include "zenoapplication.h"
#include "model/graphsmanager.h"
#include "declmetatype.h"
#include "variantptr.h"


ZGeometrySpreadsheet::ZGeometrySpreadsheet(QWidget* parent)
    : QWidget(parent)
    , m_lblNode(new QLabel)
    , m_views(new QStackedWidget(this))
    , m_vertex(new ZToolBarButton(true, ":/icons/geomsheet_vertex_idle.svg", ":/icons/geomsheet_vertex_on.svg"))
    , m_point(new ZToolBarButton(true, ":/icons/geomsheet_point_idle.svg", ":/icons/geomsheet_point_on.svg"))
    , m_face(new ZToolBarButton(true, ":/icons/geomsheet_face_idle.svg", ":/icons/geomsheet_face_on.svg"))
    , m_geom(new ZToolBarButton(true, ":/icons/geomsheet_geometry_idle.svg", ":/icons/geomsheet_geometry_on.svg"))
    , m_model(nullptr)
    , m_nodeIdx(QModelIndex())
{
    m_views->addWidget(new QTableView); //vertex
    m_views->addWidget(new QTableView); //point
    m_views->addWidget(new QTableView); //face
    m_views->addWidget(new QTableView); //geom

    QLabel* pLblBlank = new QLabel("No object available, may be not apply or result is null");
    m_views->addWidget(pLblBlank);    //blank

    QPalette palette = m_lblNode->palette();
    palette.setColor(QPalette::WindowText, Qt::white);  // 设置字体颜色为蓝色
    m_lblNode->setPalette(palette);
    pLblBlank->setPalette(palette);

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
        if (!bChecked) {
            m_vertex->setChecked(true);
            return;
        }
        m_point->setChecked(!bChecked);
        m_face->setChecked(!bChecked);
        m_geom->setChecked(!bChecked);
        m_views->setCurrentIndex(0);
        });

    connect(m_point, &ZToolBarButton::toggled, [&](bool bChecked) {
        if (!bChecked) {
            m_point->setChecked(true);
            return;
        }
        m_vertex->setChecked(!bChecked);
        m_face->setChecked(!bChecked);
        m_geom->setChecked(!bChecked);
        m_views->setCurrentIndex(1);
        });

    connect(m_face, &ZToolBarButton::toggled, [&](bool bChecked) {
        if (!bChecked) {
            m_face->setChecked(true);
            return;
        }
        m_vertex->setChecked(!bChecked);
        m_point->setChecked(!bChecked);
        m_geom->setChecked(!bChecked);
        m_views->setCurrentIndex(2);
        });

    connect(m_geom, &ZToolBarButton::toggled, [&](bool bChecked) {
        if (!bChecked) {
            m_geom->setChecked(true);
            return;
        }
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

void ZGeometrySpreadsheet::setGeometry(
        GraphModel* subgraph,
        QModelIndex nodeidx,
        std::shared_ptr<zeno::GeometryObject_Adapter> spObject
) {
    if (nodeidx.isValid()) {
        QString nodename = nodeidx.data(QtRole::ROLE_NODE_NAME).toString();
        m_lblNode->setText(nodename);
    }

    if (!spObject) { 
        m_views->setCurrentIndex(m_views->count() - 1);
        return;
    }
    else {
        if (m_vertex->isChecked())
            m_views->setCurrentIndex(0);

        if (m_point->isChecked())
            m_views->setCurrentIndex(1);

        if (m_face->isChecked())
            m_views->setCurrentIndex(2);

        if (m_geom->isChecked())
            m_views->setCurrentIndex(3);
    }

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
        model->setGeoObject(spObject);
    } else {
        view->setModel(new VertexModel(spObject));
    }

    view = qobject_cast<QTableView*>(m_views->widget(1));
    if (PointModel* model = qobject_cast<PointModel*>(view->model())) {
        model->setGeoObject(spObject);
    }
    else {
        view->setModel(new PointModel(spObject));
    }

    view = qobject_cast<QTableView*>(m_views->widget(2));
    if (FaceModel* model = qobject_cast<FaceModel*>(view->model())) {
        model->setGeoObject(spObject);
    }
    else {
        view->setModel(new FaceModel(spObject));
    }

    view = qobject_cast<QTableView*>(m_views->widget(3));
    if (GeomDetailModel* model = qobject_cast<GeomDetailModel*>(view->model())) {
        model->setGeoObject(spObject);
    }
    else {
        view->setModel(new GeomDetailModel(spObject));
    }
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
    if (topLeft.data(QtRole::ROLE_NODE_NAME).toString() == m_nodeIdx.data(QtRole::ROLE_NODE_NAME).toString()) {
        if (!roles.empty() && roles[0] == QtRole::ROLE_NODE_RUN_STATE) {
            zeno::NodeRunStatus currStatus = topLeft.data(QtRole::ROLE_NODE_RUN_STATE).value<NodeState>().runstatus;
            if (currStatus == zeno::Node_Running) {
                clearModel();
            } else if (currStatus == zeno::Node_RunSucceed) {
                zeno::zany pObject = m_nodeIdx.data(QtRole::ROLE_OUTPUT_OBJS).value<zeno::zany>();
                if (auto spGeom = std::dynamic_pointer_cast<zeno::GeometryObject_Adapter>(pObject)) {
                    setGeometry(QVariantPtr<GraphModel>::asPtr(m_nodeIdx.data(QtRole::ROLE_GRAPH)), m_nodeIdx, spGeom);
                }
            }
        }
    }
}

void ZGeometrySpreadsheet::clearModel()
{
    for (int i = 0; i < m_views->count(); ++i) {
        QTableView* view = qobject_cast<QTableView*>(m_views->widget(i));
        if (!view) continue;
        if (QAbstractItemModel* model = view->model()) {
            view->setModel(nullptr);
            delete model;
        }
    }
}

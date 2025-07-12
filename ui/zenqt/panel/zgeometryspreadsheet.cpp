#include "zgeometryspreadsheet.h"
#include "../layout/docktabcontent.h"
#include <zeno/types/IGeometryObject.h>
#include "model/geometrymodel.h"
#include "zenoapplication.h"
#include "model/graphsmanager.h"
#include "declmetatype.h"
#include "variantptr.h"
#include "zassert.h"


static void exportTableViewToCsv(QTableView* tableView) {
    QAbstractItemModel* model = tableView->model();
    if (!model) {
        QMessageBox::warning(nullptr, "错误", "表格没有模型！");
        return;
    }

    int columnCount = model->columnCount();

    // 1. 获取所有列头
    QMap<QString, int> allColumns; // key: header label, value: column index
    for (int col = 0; col < columnCount; ++col) {
        QString header = model->headerData(col, Qt::Horizontal).toString();
        allColumns.insert(header, col);
    }

    // 2. 弹出输入框，获取列名（空格分隔）
    bool ok = false;
    QString input = QInputDialog::getText(
        nullptr,
        "选择导出列",
        QString("请输入列标题，使用空格分隔（可用列：%1）").arg(allColumns.keys().join(" ")),
        QLineEdit::Normal,
        "",
        &ok
    );

    if (!ok)
        return;

    // 3. 解析用户输入，转为列表，并校验列名
    QStringList headersToExport;
    QStringList selectedHeaders = input.split(" ", Qt::SkipEmptyParts);

    if (selectedHeaders.isEmpty()) {
        // 空输入 -> 默认导出所有列
        headersToExport = allColumns.keys();
    }
    else {
        QSet<QString> validHeaders = allColumns.keys().toSet();
        for (const QString& header : selectedHeaders) {
            if (!validHeaders.contains(header)) {
                QMessageBox::warning(nullptr, "错误", QString("列 \"%1\" 不存在！").arg(header));
                return;
            }
            headersToExport << header;
        }
    }

    headersToExport.sort(Qt::CaseInsensitive);  // 按字母排序

    // 4. 选择导出文件路径
    QString filePath = QFileDialog::getSaveFileName(
        nullptr,
        "导出为 CSV",
        "",
        "CSV 文件 (*.csv)"
    );

    if (filePath.isEmpty())
        return;

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(nullptr, "错误", "无法打开文件进行写入！");
        return;
    }

    QTextStream out(&file);
    out.setCodec("UTF-8");

    // 5. 写入标题行
    out << headersToExport.join(",") << "\n";

    // 6. 写入数据行
    int rowCount = model->rowCount();
    for (int row = 0; row < rowCount; ++row) {
        QStringList rowData;
        for (const QString& header : headersToExport) {
            int col = allColumns[header];
            QVariant value = model->data(model->index(row, col));
            QString cellData;
            if (value.type() == QMetaType::Double || value.canConvert<double>()) {
                double num = value.toDouble();
                if (abs(num) > 10) {
                    cellData = QString::number(num, 'f', 2);  // 保留4位小数
                }
                else {
                    cellData = QString::number(num, 'f', 4);  // 保留4位小数
                }
            }
            else {
                cellData = value.toString();
            }

            cellData.replace('"', "\"\"");
            if (cellData.contains(',') || cellData.contains('"'))
                cellData = QString("\"%1\"").arg(cellData);
            rowData << cellData;
        }
        out << rowData.join(",") << "\n";
    }

    file.close();
    QMessageBox::information(nullptr, "完成", "导出成功！");
}



ZGeometrySpreadsheet::ZGeometrySpreadsheet(QWidget* parent)
    : QWidget(parent)
    , m_lblNode(new QLabel)
    , m_views(new QStackedWidget(this))
    , m_vertex(new ZToolBarButton(true, ":/icons/geomsheet_vertex_idle.svg", ":/icons/geomsheet_vertex_on.svg"))
    , m_point(new ZToolBarButton(true, ":/icons/geomsheet_point_idle.svg", ":/icons/geomsheet_point_on.svg"))
    , m_face(new ZToolBarButton(true, ":/icons/geomsheet_face_idle.svg", ":/icons/geomsheet_face_on.svg"))
    , m_geom(new ZToolBarButton(true, ":/icons/geomsheet_geometry_idle.svg", ":/icons/geomsheet_geometry_on.svg"))
    , m_ud(new ZToolBarButton(true, ":/icons/geo_userdata-idle.svg", ":/icons/geo_userdata-on.svg"))
    , m_model(nullptr)
    , m_nodeIdx(QModelIndex())
{
    m_views->addWidget(new QTableView); //vertex
    m_views->addWidget(new QTableView); //point
    m_views->addWidget(new QTableView); //face
    m_views->addWidget(new QTableView); //geom
    m_views->addWidget(new QTableView); //ud

    QLabel* pImgBlank = new QLabel("Current Object Is an image, please watch it in image panel");
    m_views->addWidget(pImgBlank);

    QLabel* pLblBlank = new QLabel("No object available, may be not apply or result is null");
    m_views->addWidget(pLblBlank);    //blank

    QPalette palette = m_lblNode->palette();
    palette.setColor(QPalette::WindowText, Qt::white);  // 设置字体颜色为蓝色
    m_lblNode->setPalette(palette);
    pLblBlank->setPalette(palette);
    pImgBlank->setPalette(palette);

    QHBoxLayout* pToolbarLayout = new QHBoxLayout;

    QLineEdit* pJumpNum = new QLineEdit;
    pJumpNum->setFixedWidth(56);
    pJumpNum->setProperty("cssClass", "zeno2_2_lineedit");
    QPushButton* pJumpBtn = new QPushButton(tr("Jump"));
    pJumpBtn->setProperty("cssClass", "proppanel");
    pJumpBtn->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

    connect(pJumpBtn, &QPushButton::clicked, [=]() {
        bool bOk = false;
        int line = pJumpNum->text().toInt(&bOk);
        if (m_point->isChecked()) {
            QTableView* pTableView = qobject_cast<QTableView*>(m_views->currentWidget());
            QModelIndex index = pTableView->model()->index(line, 0);
            if (index.isValid()) {
                pTableView->scrollTo(index, QAbstractItemView::PositionAtTop);
            }
        }
    });

    QPushButton* pExport = new QPushButton(tr("Export"));
    pExport->setProperty("cssClass", "proppanel");
    pExport->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    connect(pExport, &QPushButton::clicked, [=]() {
        if (m_point->isChecked()) {
            QTableView* pTableView = qobject_cast<QTableView*>(m_views->currentWidget());
            ZASSERT_EXIT(pTableView);
            exportTableViewToCsv(pTableView);
        }
    });


    pToolbarLayout->addWidget(m_lblNode);

    pToolbarLayout->addWidget(pJumpNum);
    pToolbarLayout->addWidget(pJumpBtn);
    pToolbarLayout->addWidget(pExport);

    pToolbarLayout->addWidget(m_vertex);
    pToolbarLayout->addWidget(m_point);
    pToolbarLayout->addWidget(m_face);
    pToolbarLayout->addWidget(m_geom);
    pToolbarLayout->addWidget(m_ud);

    QVBoxLayout* pMainLayout = new QVBoxLayout;
    pMainLayout->addLayout(pToolbarLayout);
    pMainLayout->addWidget(m_views);

    m_vertex->setChecked(false);
    m_point->setChecked(true);
    m_face->setChecked(false);
    m_geom->setChecked(false);
    m_ud->setChecked(false);

    connect(m_vertex, &ZToolBarButton::toggled, [&](bool bChecked) {
        if (!bChecked) {
            m_vertex->setChecked(true);
            return;
        }
        m_point->setChecked(!bChecked);
        m_face->setChecked(!bChecked);
        m_geom->setChecked(!bChecked);
        m_ud->setChecked(!bChecked);
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
        m_ud->setChecked(!bChecked);
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
        m_ud->setChecked(!bChecked);
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
        m_ud->setChecked(!bChecked);
        m_views->setCurrentIndex(3);
        });

    connect(m_ud, &ZToolBarButton::toggled, [&](bool bChecked) {
        if (!bChecked) {
            m_ud->setChecked(true);
            return;
        }
        m_vertex->setChecked(!bChecked);
        m_face->setChecked(!bChecked);
        m_point->setChecked(!bChecked);
        m_geom->setChecked(!bChecked);
        m_views->setCurrentIndex(4);
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
        //unavailable page
        m_views->setCurrentIndex(m_views->count() - 1);
        return;
    }
    else {
        if (spObject->userData()->has("isImage")) {
            m_views->setCurrentIndex(m_views->count() - 2);
            return;
        }

        if (m_vertex->isChecked())
            m_views->setCurrentIndex(0);

        if (m_point->isChecked())
            m_views->setCurrentIndex(1);

        if (m_face->isChecked())
            m_views->setCurrentIndex(2);

        if (m_geom->isChecked())
            m_views->setCurrentIndex(3);

        if (m_ud->isChecked())
            m_views->setCurrentIndex(4);
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

    view = qobject_cast<QTableView*>(m_views->widget(4));
    if (GeomUserDataModel* udmodel = qobject_cast<GeomUserDataModel*>(view->model())) {
        udmodel->setGeoObject(spObject);
    }
    else {
        view->setModel(new GeomUserDataModel(spObject));
    }
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

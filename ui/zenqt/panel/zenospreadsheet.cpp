//
// Created by zh on 2022/6/27.
//

#include "zenospreadsheet.h"
#include "PrimAttrTableModel.h"
#include "viewport/zenovis.h"
#include "zeno/utils/format.h"
#include <zeno/types/UserData.h>
#include <zeno/types/PrimitiveObject.h>
#include "widgets/zcombobox.h"
#include "zenomainwindow.h"
#include "viewport/viewportwidget.h"
#include "viewport/displaywidget.h"
#include "nodeeditor/gv/zenographseditor.h"
#include "settings/zenosettingsmanager.h"
#include "nodeeditor/gv/zenosubgraphscene.h"
#include "zenoapplication.h"
#include "zassert.h"
#include "model/graphsmanager.h"


ZenoSpreadsheet::ZenoSpreadsheet(QWidget *parent) : QWidget(parent) {
    dataModel = new PrimAttrTableModel();
    QVBoxLayout* pMainLayout = new QVBoxLayout;
    pMainLayout->setContentsMargins(QMargins(0, 0, 0, 0));
    setLayout(pMainLayout);
    setFocusPolicy(Qt::ClickFocus);

    QPalette palette = this->palette();
    palette.setBrush(QPalette::Window, QColor(37, 37, 38));
    setPalette(palette);
    setAutoFillBackground(true);

    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);

    QHBoxLayout* pTitleLayout = new QHBoxLayout;

    QLabel* pPrim = new QLabel(tr("Prim: "));
    pPrim->setProperty("cssClass", "proppanel");
    pTitleLayout->addWidget(pPrim);

    pPrimName->setProperty("cssClass", "proppanel");
    pTitleLayout->addWidget(pPrimName);

    m_checkSortingEnabled = new QCheckBox(this);
    m_checkSortingEnabled->setProperty("cssClass", "proppanel");
    m_checkSortingEnabled->setText(tr("enable sort"));
    pTitleLayout->addWidget(m_checkSortingEnabled);

    ZComboBox* pMode = new ZComboBox();
    pMode->addItem("Vertex");
    pMode->addItem("Tris");
    pMode->addItem("Points");
    pMode->addItem("Lines");
    pMode->addItem("Quads");
    pMode->addItem("Polys");
    pMode->addItem("Loops");
    pMode->addItem("UVs");
    pMode->addItem("UserData");
    pMode->setProperty("cssClass", "proppanel");
    pTitleLayout->addWidget(pMode);

    pMainLayout->addLayout(pTitleLayout);

    auto sortModel = new QSortFilterProxyModel(this);
    sortModel->setSourceModel(dataModel);

    prim_attr_view = new QTableView();
    prim_attr_view->setAlternatingRowColors(true);
    prim_attr_view->setSortingEnabled(false);
    prim_attr_view->setProperty("cssClass", "proppanel");
    prim_attr_view->setModel(sortModel);
    prim_attr_view->installEventFilter(this);
    pMainLayout->addWidget(prim_attr_view);

//    pStatusBar->setAlignment(Qt::AlignRight);
    pStatusBar->setProperty("cssClass", "proppanel");
    pMainLayout->addWidget(pStatusBar);

    ZenoMainWindow *pWin = zenoApp->getMainWindow();
    ZERROR_EXIT(pWin);

    connect(pWin, &ZenoMainWindow::visObjectsUpdated, this, [=](ViewportWidget* pViewport, int frame) {
        return;
    });

    connect(pMode, &ZComboBox::_textActivated, [=](const QString& text) {
        this->dataModel->setSelAttr(text.toStdString());
        // do not sort for userdata
        if (text == "UserData") {
            // reset sort order
            sortModel->sort(-1);
            prim_attr_view->horizontalHeader()->setSortIndicator(-1, Qt::SortOrder::AscendingOrder);
            prim_attr_view->setSortingEnabled(false);
        }
        else {
            prim_attr_view->setSortingEnabled(m_checkSortingEnabled->checkState());
        }
    });

    // enable sort
    connect(m_checkSortingEnabled, &QCheckBox::stateChanged, this, [this](int state) {
        prim_attr_view->setSortingEnabled(state != Qt::CheckState::Unchecked);
    });

    // corner button of tableview
    auto cornerBtn = prim_attr_view->findChild<QAbstractButton*>();
    // do not select all when clicked
    cornerBtn->disconnect();
    // reset sort order
    connect(cornerBtn, &QAbstractButton::clicked, this, [&, sortModel]() {
        sortModel->sort(-1);
        prim_attr_view->horizontalHeader()->setSortIndicator(-1, Qt::SortOrder::AscendingOrder);
    });
}

void ZenoSpreadsheet::clear() {
    pPrimName->clear();
    this->dataModel->setModelData(nullptr);
    pStatusBar->clear();
}

void ZenoSpreadsheet::onNodeSelected(const QModelIndex& idx) {
    ZASSERT_EXIT(idx.isValid());
    zeno::zany pObject = idx.data(ROLE_OUTPUT_OBJS).value<zeno::zany>();
    if (!pObject)
    {
        this->dataModel->setModelData(nullptr);
        return;
    }

    if (auto obj = dynamic_cast<zeno::PrimitiveObject*>(pObject.get())) {
        size_t sizeUserData = obj->userData().size();
        size_t num_attrs = obj->num_attrs();
        size_t num_vert = obj->verts.size();
        size_t num_tris = obj->tris.size();
        size_t num_loops = obj->loops.size();
        size_t num_polys = obj->polys.size();
        size_t num_lines = obj->lines.size();

        QString statusInfo = QString("Vertex: %1, Triangle: %2, Loops: %3, Poly: %4, Lines: %5, UserData: %6, Attribute: %7")
            .arg(num_vert)
            .arg(num_tris)
            .arg(num_loops)
            .arg(num_polys)
            .arg(num_lines)
            .arg(sizeUserData)
            .arg(num_attrs);
        pStatusBar->setText(statusInfo);
        this->dataModel->setModelData(obj);
    }
}

void ZenoSpreadsheet::setPrim(std::string primid) {
    //TODO: 
    return;
}

bool ZenoSpreadsheet::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == prim_attr_view && event->type() == QEvent::KeyPress)
    {
        QKeyEvent* keyEvt = dynamic_cast<QKeyEvent*>(event);
        if (keyEvt == QKeySequence::Copy) {
            if (QItemSelectionModel* pSelectionModel = prim_attr_view->selectionModel())
            {
                int cols = dataModel->columnCount(QModelIndex());
                const QModelIndexList& lst = pSelectionModel->selectedIndexes();
                QString copyStr;
                QList<int> rowsExist;
                for (const auto& idx : lst)
                {
                    int row = idx.row();
                    if (rowsExist.contains(row))
                        continue;
                    rowsExist << row;
                    QString str;
                    for (int col = 0; col < cols; col++)
                    {
                        const QModelIndex index = prim_attr_view->model()->index(row, col);
                        if (lst.contains(index))
                        {
                            if (!str.isEmpty())
                                str += ":";
                            str += index.data(Qt::DisplayRole).toString();
                        }

                    }
                    if (!str.isEmpty())
                    {
                        if (!copyStr.isEmpty())
                            copyStr += ",";
                        copyStr += str;
                    }
                }
                if (!copyStr.isEmpty())
                {
                    QMimeData* pMimeData = new QMimeData;
                    pMimeData->setText(copyStr);
                    QApplication::clipboard()->setMimeData(pMimeData);
                    return true;
                }
            }
        }
    }
    return QWidget::eventFilter(watched, event);
}


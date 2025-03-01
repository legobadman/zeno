#include "zcurvemapeditor.h"
#include "ui_zcurvemapeditor.h"
#include "curvemapview.h"
#include "style/zenostyle.h"
#include "curvenodeitem.h"
#include "model/curvemodel.h"
#include "curvesitem.h"
#include "util/uihelper.h"
#include "widgets/effect/innershadoweffect.h"
#include "zassert.h"
#include "model/graphsmanager.h"
#include "variantptr.h"
#include "util/curveutil.h"


ZCurveMapEditor::ZCurveMapEditor(bool bTimeline, QWidget* parent)
	: QDialog(parent)
    , m_pGroupHdlType(nullptr)
    , m_channelModel(nullptr)
    , m_bTimeline(false)
{
    initUI();
    initChannelModel();
    init();
}

ZCurveMapEditor::~ZCurveMapEditor()
{
}

void ZCurveMapEditor::initUI()
{
    m_ui = new Ui::FCurveDlg;
    m_ui->setupUi(this);

    setProperty("cssClass", "F-Curve");

    m_ui->btnLoadPreset->setProperty("cssClass", "curve-preset");
    m_ui->btnSavePreset->setProperty("cssClass", "curve-preset");
    m_ui->cbIsTimeline->setProperty("cssClass", "curve-timeline");
    m_ui->btnAddCurve->setProperty("cssClass", "curve-preset");
    m_ui->btnDelCurve->setProperty("cssClass", "curve-preset");

    //todo: move to ui file.
    m_pGroupHdlType = new QButtonGroup(this);
    m_pGroupHdlType->setExclusive(true);
    m_pGroupHdlType->addButton(m_ui->btnFree, HDL_FREE);
    m_pGroupHdlType->addButton(m_ui->btnAligned, HDL_ALIGNED);
    m_pGroupHdlType->addButton(m_ui->btnVector, HDL_VECTOR);
    m_pGroupHdlType->addButton(m_ui->btnAsymmetry, HDL_ASYM);

    m_ui->btnLockX->setIcons(ZenoStyle::dpiScaledSize(QSize(17, 17)),
                             ":/icons/ic_tool_unlock.svg", "", ":/icons/ic_tool_lock.svg");
    m_ui->btnLockY->setIcons(ZenoStyle::dpiScaledSize(QSize(17, 17)),
                             ":/icons/ic_tool_unlock.svg", "", ":/icons/ic_tool_lock.svg");

    initStylesheet();
    initButtonShadow();
    initSize();
    initSignals();
}

void ZCurveMapEditor::initSize()
{
    //qt designer doesn't support dpi scaled size.
    m_ui->editXFrom->setMinimumSize(ZenoStyle::dpiScaledSize(QSize(60, 24)));
    m_ui->editXTo->setMinimumSize(ZenoStyle::dpiScaledSize(QSize(60, 24)));
    m_ui->editYFrom->setMinimumSize(ZenoStyle::dpiScaledSize(QSize(60, 24)));
    m_ui->editYTo->setMinimumSize(ZenoStyle::dpiScaledSize(QSize(60, 24)));

    m_ui->editPtX->setMinimumSize(ZenoStyle::dpiScaledSize(QSize(60, 24)));
    m_ui->editPtY->setMinimumSize(ZenoStyle::dpiScaledSize(QSize(60, 24)));
    m_ui->editFrame->setMinimumSize(ZenoStyle::dpiScaledSize(QSize(60, 24)));
    m_ui->editValue->setMinimumSize(ZenoStyle::dpiScaledSize(QSize(60, 24)));

    m_ui->editTanLeftX->setMinimumSize(ZenoStyle::dpiScaledSize(QSize(65, 24)));
    m_ui->editTanLeftY->setMinimumSize(ZenoStyle::dpiScaledSize(QSize(65, 24)));
    m_ui->editTanRightX->setMinimumSize(ZenoStyle::dpiScaledSize(QSize(65, 24)));
    m_ui->editTanRightY->setMinimumSize(ZenoStyle::dpiScaledSize(QSize(65, 24)));

    QSize size = ZenoStyle::dpiScaledSize(QSize(35, 24));
    m_ui->btnVector->setFixedSize(size);
    m_ui->btnAsymmetry->setFixedSize(size);
    m_ui->btnAligned->setFixedSize(size);
    m_ui->btnFree->setFixedSize(size);

    m_ui->widget->setFixedWidth(ZenoStyle::dpiScaled(180));
    size = QSize(ZenoStyle::dpiScaled(1000), ZenoStyle::dpiScaled(500));
    resize(size);
}

void ZCurveMapEditor::initStylesheet()
{
    auto editors = findChildren<QLineEdit*>(QString(), Qt::FindDirectChildrenOnly);
    for (QLineEdit* pLineEdit : editors)
    {
        pLineEdit->setProperty("cssClass", "FCurve-lineedit");
    }
    m_ui->editPtX->setProperty("cssClass", "FCurve-lineedit");
}

void ZCurveMapEditor::initButtonShadow()
{
    auto btnList = findChildren<QPushButton*>();
    for (QPushButton* btn : btnList)
    {
        InnerShadowEffect *effect = new InnerShadowEffect;
        btn->setGraphicsEffect(effect);
    }
}

void ZCurveMapEditor::initChannelModel()
{
    m_channelModel = new QStandardItemModel(this);
    QStandardItem *pRootItem = new QStandardItem("Channels");
    m_channelModel->appendRow(pRootItem);
    m_ui->channelView->setModel(m_channelModel);
    m_ui->channelView->expandAll();

    m_selection = new QItemSelectionModel(m_channelModel);
    //m_ui->channelView->setVisible(m_ui->cbIsTimeline->isChecked());
}

CurveModel* ZCurveMapEditor::currentModel()
{
    auto lst = m_ui->gridview->getSelectedNodes();
    if (lst.size() == 0)
        return nullptr;
    return lst[0]->curves()->model();
}

void ZCurveMapEditor::init()
{
    m_ui->gridview->init(m_ui->cbIsTimeline->isChecked());

    auto range = m_ui->gridview->range();
    m_ui->editXFrom->setText(QString::number(range.xFrom));
    m_ui->editXFrom->setValidator(new QDoubleValidator);
    m_ui->editXTo->setText(QString::number(range.xTo));
    m_ui->editXTo->setValidator(new QDoubleValidator);
    m_ui->editYFrom->setText(QString::number(range.yFrom));
    m_ui->editYFrom->setValidator(new QDoubleValidator);
    m_ui->editYTo->setText(QString::number(range.yTo));
    m_ui->editYTo->setValidator(new QDoubleValidator);

    connect(m_ui->editPtX, SIGNAL(editingFinished()), this, SLOT(onLineEditFinished()));
    connect(m_ui->editPtY, SIGNAL(editingFinished()), this, SLOT(onLineEditFinished()));
    connect(m_ui->editTanLeftX, SIGNAL(editingFinished()), this, SLOT(onLineEditFinished()));
    connect(m_ui->editTanLeftY, SIGNAL(editingFinished()), this, SLOT(onLineEditFinished()));
    connect(m_ui->editTanRightX, SIGNAL(editingFinished()), this, SLOT(onLineEditFinished()));
    connect(m_ui->editTanRightY, SIGNAL(editingFinished()), this, SLOT(onLineEditFinished()));
}

void ZCurveMapEditor::initSignals()
{
    connect(m_pGroupHdlType, SIGNAL(buttonToggled(QAbstractButton *, bool)), this, SLOT(onButtonToggled(QAbstractButton*, bool)));
    connect(m_ui->gridview, &CurveMapView::nodeItemsSelectionChanged, this, &ZCurveMapEditor::onNodesSelectionChanged);
    connect(m_ui->gridview, &CurveMapView::frameChanged, this, &ZCurveMapEditor::onFrameChanged);
    connect(m_ui->btnLockX, SIGNAL(toggled(bool)), this, SLOT(onLockBtnToggled(bool)));
    connect(m_ui->btnLockY, SIGNAL(toggled(bool)), this, SLOT(onLockBtnToggled(bool)));
    connect(m_ui->editXFrom, SIGNAL(editingFinished()), this, SLOT(onRangeEdited()));
    connect(m_ui->editXTo, SIGNAL(editingFinished()), this, SLOT(onRangeEdited()));
    connect(m_ui->editYFrom, SIGNAL(editingFinished()), this, SLOT(onRangeEdited()));
    connect(m_ui->editYTo, SIGNAL(editingFinished()), this, SLOT(onRangeEdited()));
    connect(m_ui->cbIsTimeline, SIGNAL(stateChanged(int)), this, SLOT(onCbTimelineChanged(int)));
    connect(m_ui->btnAddCurve, SIGNAL(clicked()), this, SLOT(onAddCurveBtnClicked()));
    connect(m_ui->btnDelCurve, SIGNAL(clicked()), this, SLOT(onDelCurveBtnClicked()));
}

void ZCurveMapEditor::addCurves(const zeno::CurvesData& curves)
{
    for (auto& [key, curve] : curves.keys)
    {
        CurveModel* model = new CurveModel(QString::fromStdString(key), curve.rg, this);
        model->initItems(curve);
        model->setVisible(curve.visible);
        model->setTimeline(curve.timeline);
        addCurve(model);
    }
}

void ZCurveMapEditor::addCurve(CurveModel *model)
{
    //static const QColor preset[] = {"#CE2F2F", "#2FCD5F", "#307BCD"};
    static const QMap<QString, QColor> preset = {{"x", "#CE2F2F"}, {"y", "#2FCD5F"}, {"z", "#307BCD"}};

    QString id = model->id();
    m_models.insert(id, model);
    m_ui->gridview->addCurve(model);

    m_ui->cbIsTimeline->setChecked(model->isTimeline());

    auto range = m_ui->gridview->range();
    m_ui->editXFrom->setText(QString::number(range.xFrom));
    m_ui->editXTo->setText(QString::number(range.xTo));
    m_ui->editYFrom->setText(QString::number(range.yFrom));
    m_ui->editYTo->setText(QString::number(range.yTo));

    //int n = pRootItem->rowCount();
    //QColor curveClr;
    //if (n < sizeof(preset) / sizeof(QColor)) 
    //{
    //    curveClr = preset[n];
    //}
    //else
    //{
    //    curveClr = QColor(77, 77, 77);
    //}

    m_bate_rows.push_back(model);
    CurveGrid *pGrid = m_ui->gridview->gridItem();
    QColor col;
    if (!preset.contains(id))
        col = QColor("#CE2F2F");
    else
        col = preset[id];
    pGrid->setCurvesColor(id, col);
    pGrid->setCurvesVisible(id, model->getVisible());

    QStandardItem *pItem = new QStandardItem(model->id());
    pItem->setCheckable(true);
    pItem->setCheckState(model->getVisible() ? Qt::Checked : Qt::Unchecked);
    QStandardItem *pRootItem = m_channelModel->itemFromIndex(m_channelModel->index(0, 0));
    if (pRootItem->rowCount() == 0)
    {
        pRootItem->appendRow(pItem);
    }
    else {
        int i = 0;
        while (pRootItem->child(i, 0) != NULL && model->id() > pRootItem->child(i, 0)->data(Qt::DisplayRole).toString())
        {
            i++;
        }
        pRootItem->insertRow(i, pItem);
    }

    connect(model, &CurveModel::dataChanged, this, &ZCurveMapEditor::onNodesDataChanged);
    connect(m_channelModel, &QStandardItemModel::dataChanged, this, &ZCurveMapEditor::onChannelModelDataChanged);
}

void ZCurveMapEditor::onRangeEdited()
{
    zeno::CurveData::Range newRg = {m_ui->editXFrom->text().toDouble(), m_ui->editXTo->text().toDouble(),
                         m_ui->editYFrom->text().toDouble(), m_ui->editYTo->text().toDouble()};
    auto rg = m_ui->gridview->range();
    if (rg.xFrom != newRg.xFrom || rg.xTo != newRg.xTo || rg.yFrom != newRg.yFrom || rg.yTo != newRg.yTo)
    {
        m_ui->gridview->resetRange(newRg);
    }
}

void ZCurveMapEditor::onCbTimelineChanged(int state)
{
    if (state == Qt::Checked) {
        m_ui->gridview->setChartType(true);
    } else if (state == Qt::Unchecked) {
        m_ui->gridview->setChartType(false);
    }
    for (CurveModel* model : m_models)
    {
        model->setTimeline(state == Qt::Checked);
    }
}

void ZCurveMapEditor::onAddCurveBtnClicked() {
    QStandardItem * pRootItem = m_channelModel->itemFromIndex(m_channelModel->index(0, 0));
    if (pRootItem->rowCount() != 3)
    {
        CurveModel *newCurve = curve_util::deflModel(this);

        if (pRootItem->child(0, 0) == NULL || pRootItem->child(0, 0)->data(Qt::DisplayRole) != "x")
        {
            newCurve->setId("x");
            newCurve->setData(newCurve->index(0, 0), QVariant::fromValue(QPointF(0, 0)), ROLE_NODEPOS);
            newCurve->setData(newCurve->index(1, 0), QVariant::fromValue(QPointF(1, 1)), ROLE_NODEPOS);
            addCurve(newCurve);
        } else if (pRootItem->child(1, 0) == NULL || pRootItem->child(1, 0)->data(Qt::DisplayRole) != "y")
        {
            newCurve->setData(newCurve->index(0, 0), QVariant::fromValue(QPointF(0, 0.5)), ROLE_NODEPOS);
            newCurve->setData(newCurve->index(1, 0), QVariant::fromValue(QPointF(1, 0.5)), ROLE_NODEPOS);
            newCurve->setId("y");
            addCurve(newCurve);
        } else {
            newCurve->setData(newCurve->index(0, 0), QVariant::fromValue(QPointF(0, 1)), ROLE_NODEPOS);
            newCurve->setData(newCurve->index(1, 0), QVariant::fromValue(QPointF(1, 0)), ROLE_NODEPOS);
            newCurve->setId("z");
            addCurve(newCurve);
        }
    }
}

void ZCurveMapEditor::onDelCurveBtnClicked() {
    QModelIndexList lst = m_ui->channelView->selectionModel()->selectedIndexes();
    if (lst.size() != 0 && lst[0] != m_channelModel->index(0, 0))
    {
        QStandardItem *pRootItem = m_channelModel->itemFromIndex(m_channelModel->index(0, 0));
        QString curveName = lst[0].data(Qt::DisplayRole).toString();
        pRootItem->removeRow(lst[0].row());

        CurveGrid *pGrid = m_ui->gridview->gridItem();
        pGrid->removeCurve(curveName);

        delete m_models[curveName];
        m_bate_rows.erase(std::find(m_bate_rows.begin(), m_bate_rows.end(), m_models[curveName]));
        m_models.remove(curveName);
    }
}

int ZCurveMapEditor::curveCount() const {
    return (int)m_bate_rows.size();
}

CurveModel *ZCurveMapEditor::getCurve(int i) const {
    return m_bate_rows.at(i);
}

CURVES_MODEL ZCurveMapEditor::getModel() const {
    return m_models;
}

zeno::CurvesData ZCurveMapEditor::curves() const
{
    zeno::CurvesData curves;
    for (QString key : m_models.keys())
    {
        zeno::CurveData data = m_models[key]->getItems();
        data.visible = m_models[key]->getVisible();
        data.timeline = m_models[key]->isTimeline();
        curves.keys.insert(std::make_pair(key.toStdString(), data));
    }
    return curves;
}

void ZCurveMapEditor::onButtonToggled(QAbstractButton* btn, bool bToggled)
{
    CurveModel *pModel = currentModel();
    if (!bToggled || !pModel)
        return;

    auto lst = m_ui->gridview->getSelectedNodes();
    if (lst.size() == 1)
    {
        CurveNodeItem* node = lst[0];
        QModelIndex idx = node->index();
        ZASSERT_EXIT(idx.isValid());

        if (btn == m_ui->btnVector)
        {
            pModel->setData(idx, HDL_VECTOR, ROLE_TYPE);
        }
        else if (btn == m_ui->btnAligned)
        {
            pModel->setData(idx, HDL_ALIGNED, ROLE_TYPE);
        }
        else if (btn == m_ui->btnAsymmetry)
        {
            pModel->setData(idx, HDL_ASYM, ROLE_TYPE);
        }
        else if (btn == m_ui->btnFree)
        {
            pModel->setData(idx, HDL_FREE, ROLE_TYPE);
        }
    }
}

void ZCurveMapEditor::onLineEditFinished()
{
    CurveModel* pModel = currentModel();
    if (!pModel)
        return;

    QObject *pEdit = sender();
    if (pEdit == m_ui->editPtX) {
    
    }

    CurveGrid *pGrid = m_ui->gridview->gridItem();
    auto lst = m_ui->gridview->getSelectedNodes();
    if (lst.size() == 1)
    {
        CurveNodeItem* node = lst[0];
        QPointF logicPos = QPointF(m_ui->editPtX->text().toFloat(), m_ui->editPtY->text().toFloat());
        qreal leftX = m_ui->editTanLeftX->text().toFloat();
        qreal leftY = m_ui->editTanLeftY->text().toFloat();
        qreal rightX = m_ui->editTanRightX->text().toFloat();
        qreal rightY = m_ui->editTanRightY->text().toFloat();
        QPointF leftHdlLogic = logicPos + QPointF(leftX, leftY);
        QPointF rightHdlLogic = logicPos + QPointF(rightX, rightY);

        QPointF nodeScenePos = pGrid->logicToScene(logicPos);
        QPointF leftHdlScene = pGrid->logicToScene(leftHdlLogic);
        QPointF rightHdlScene = pGrid->logicToScene(rightHdlLogic);
        QPointF leftHdlOffset = leftHdlScene - nodeScenePos;
        QPointF rightHdlOffset = rightHdlScene - nodeScenePos;

        const QModelIndex& idx = node->index();
        pModel->setData(idx, logicPos, ROLE_NODEPOS);
        pModel->setData(idx, QPointF(leftX, leftY), ROLE_LEFTPOS);
        pModel->setData(idx, QPointF(rightX, rightY), ROLE_RIGHTPOS);
    }
}

void ZCurveMapEditor::onNodesDataChanged()
{
    CurveGrid *pGrid = m_ui->gridview->gridItem();
    auto lst = m_ui->gridview->getSelectedNodes();
    bool enableEditor = lst.size() == 1;
    m_ui->editPtX->setEnabled(enableEditor);
    m_ui->editPtX->setText("");
    m_ui->editPtY->setEnabled(enableEditor);
    m_ui->editPtY->setText("");
    m_ui->editTanLeftX->setEnabled(enableEditor);
    m_ui->editTanLeftX->setText("");
    m_ui->editTanLeftY->setEnabled(enableEditor);
    m_ui->editTanLeftY->setText("");
    m_ui->editTanRightX->setEnabled(enableEditor);
    m_ui->editTanRightX->setText("");
    m_ui->editTanRightY->setEnabled(enableEditor);
    m_ui->editTanRightY->setText("");
    m_ui->btnLockX->setEnabled(enableEditor);
    m_ui->btnLockX->toggle(false);
    m_ui->btnLockX->setText("");
    m_ui->btnLockY->setEnabled(enableEditor);
    m_ui->btnLockY->toggle(false);
    m_ui->btnLockY->setText("");
   
    if (lst.size() == 1)
    {
        ZASSERT_EXIT(pGrid);
        CurveNodeItem *node = lst[0];
        const QModelIndex& idx = node->index();
        QPointF logicPos = idx.data(ROLE_NODEPOS).toPointF();
        m_ui->editPtX->setText(QString::number(logicPos.x(), 'g', 3));
        m_ui->editPtY->setText(QString::number(logicPos.y(), 'g', 3));

        QPointF leftPos = idx.data(ROLE_LEFTPOS).toPointF();
        QPointF rightPos = idx.data(ROLE_RIGHTPOS).toPointF();

        bool bLockX = idx.data(ROLE_LOCKX).toBool();
        bool bLockY = idx.data(ROLE_LOCKY).toBool();

        m_ui->editTanLeftX->setText(QString::number(leftPos.x(), 'g', 3));
        m_ui->editTanLeftY->setText(QString::number(leftPos.y() , 'g', 3));
        m_ui->editTanRightX->setText(QString::number(rightPos.x(), 'g', 3));
        m_ui->editTanRightY->setText(QString::number(rightPos.y(), 'g', 3));

        BlockSignalScope scope1(m_ui->btnAsymmetry);
        BlockSignalScope scope2(m_ui->btnAligned);
        BlockSignalScope scope3(m_ui->btnFree);
        BlockSignalScope scope4(m_ui->btnVector);
        BlockSignalScope scope(m_pGroupHdlType);
        BlockSignalScope scope_(m_ui->btnLockX);
        BlockSignalScope scope__(m_ui->btnLockY);

        m_ui->btnLockX->toggle(bLockX);
        m_ui->btnLockY->toggle(bLockY);

        switch (idx.data(ROLE_TYPE).toInt())
        {
            case HDL_ASYM:
            {
                m_ui->btnAsymmetry->setChecked(true);
                break;
            }
            case HDL_ALIGNED:
            {
                m_ui->btnAligned->setChecked(true);
                break;
            }
            case HDL_FREE:
            {
                m_ui->btnFree->setChecked(true);
                break;
            }
            case HDL_VECTOR:
            {
                m_ui->btnVector->setChecked(true);
                break;
            }
        }
    }
}

void ZCurveMapEditor::onLockBtnToggled(bool bToggle)
{
    if (sender() != m_ui->btnLockX && sender() != m_ui->btnLockY)
        return;

    auto lst = m_ui->gridview->getSelectedNodes();
    if (lst.size() == 1)
    {
        CurveNodeItem *node = lst[0];
        QModelIndex idx = node->index();
        ZASSERT_EXIT(idx.isValid());
        CurveModel *pModel = currentModel();
        if (pModel)
        {
            pModel->setData(idx, bToggle, sender() == m_ui->btnLockX ? ROLE_LOCKX : ROLE_LOCKY);
        }
    }
}

void ZCurveMapEditor::onNodesSelectionChanged(QList<CurveNodeItem*> lst)
{
    onNodesDataChanged();
}

void ZCurveMapEditor::onChannelModelDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    if (!topLeft.isValid() || roles.isEmpty())
        return;

    QString id = topLeft.data(Qt::DisplayRole).toString();
    if (roles[0] == Qt::CheckStateRole)
    {
        ZASSERT_EXIT(m_models.find(id) != m_models.end());

        CurveGrid* pGrid = m_ui->gridview->gridItem();
        ZASSERT_EXIT(pGrid);

        Qt::CheckState state = topLeft.data(Qt::CheckStateRole).value<Qt::CheckState>();
        if (state == Qt::Checked)
        {
            pGrid->setCurvesVisible(id, true);
            m_models[id]->setVisible(true);
        }
        else if (state == Qt::Unchecked)
        {
            pGrid->setCurvesVisible(id, false);
            m_models[id]->setVisible(false);
        }
    }
}

void ZCurveMapEditor::onFrameChanged(qreal frame)
{
    m_ui->editFrame->setText(QString::number(frame));
}

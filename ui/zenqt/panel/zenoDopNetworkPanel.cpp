#include "zenoDopNetworkPanel.h"
#include "widgets/zexpandablesection.h"
#include "widgets/zlabel.h"
#include "widgets/zlineedit.h"
#include "style/zenostyle.h"
#include "nodeeditor/gv/zitemfactory.h"
#include "model/GraphModel.h"
#include "variantptr.h"


zenoDopNetworkPanel::zenoDopNetworkPanel(QWidget* inputsWidget, QWidget *parent, QPersistentModelIndex& idx)
    : QTabWidget(parent)
    , m_nodeIdx(idx)
{
    int defaultMemSize = m_nodeIdx.data(QtRole::ROLE_DOPNETWORK_MEM).toInt();
    int defaultMaxMemSize = m_nodeIdx.data(QtRole::ROLE_DOPNETWORK_MAXMEM).toInt();
    bool enableCache = m_nodeIdx.data(QtRole::ROLE_DOPNETWORK_ENABLECACHE).toBool();
    bool allowCacheToDisk = m_nodeIdx.data(QtRole::ROLE_DOPNETWORK_CACHETODISK).toBool();
    this->insertTab(0, inputsWidget, "inputs");

    ZScrollArea* scrollArea = new ZScrollArea(this);
    scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    scrollArea->setMinimumHeight(0);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setWidgetResizable(true);
    ZContentWidget* pWidget = new ZContentWidget(scrollArea);
    pWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    scrollArea->setWidget(pWidget);
    QGridLayout* pLayout = new QGridLayout(pWidget);
    pLayout->setContentsMargins(10, 15, 10, 15);
    pLayout->setAlignment(Qt::AlignTop);
    pLayout->setColumnStretch(2, 3);
    pLayout->setSpacing(10);

    QFont font = QApplication::font();
    font.setWeight(QFont::Light);

    QCheckBox* enableCacheCheckBox = new QCheckBox(pWidget);
    enableCacheCheckBox->setChecked(enableCache);
    ZTextLabel* enableCacheLabel = new ZTextLabel("Enable Cache");
    enableCacheLabel->setFont(font);
    enableCacheLabel->setTextColor(QColor(255, 255, 255, 255 * 0.7));
    enableCacheLabel->setHoverCursor(Qt::ArrowCursor);
    pLayout->addWidget(enableCacheLabel, 0, 0, Qt::AlignLeft | Qt::AlignVCenter);
    pLayout->addWidget(enableCacheCheckBox, 0, 1, Qt::AlignVCenter);
    connect(enableCacheCheckBox, &QCheckBox::stateChanged, [this](int state) {
        bool bChecked = (state == Qt::Checked);
        if (GraphModel* pModel = QVariantPtr<GraphModel>::asPtr(m_nodeIdx.data(QtRole::ROLE_GRAPH)))
        {
            pModel->setData(m_nodeIdx, QtRole::ROLE_DOPNETWORK_ENABLECACHE, bChecked);
        }
    });

    ZLineEdit* cacheMemoryLineEdit = new ZLineEdit(QString::number(defaultMemSize));
    cacheMemoryLineEdit->setFixedHeight(ZenoStyle::dpiScaled(zenoui::g_ctrlHeight));
    cacheMemoryLineEdit->setProperty("cssClass", "zeno2_2_lineedit");
    QIntValidator* intValidator = new QIntValidator(1, INT_MAX, cacheMemoryLineEdit);
    cacheMemoryLineEdit->setValidator(intValidator);
    ZTextLabel* cacheMemoryLabel = new ZTextLabel("Cache Memory");
    cacheMemoryLabel->setFont(font);
    cacheMemoryLabel->setTextColor(QColor(255, 255, 255, 255 * 0.7));
    cacheMemoryLabel->setHoverCursor(Qt::ArrowCursor);
    QSlider* pSlider = new QSlider(Qt::Horizontal);
    pSlider->setStyleSheet(ZenoStyle::dpiScaleSheet("\
                    QSlider::groove:horizontal {\
                        height: 4px;\
                        background: #707D9C;\
                    }\
                    \
                    QSlider::handle:horizontal {\
                        background: #DFE2E5;\
                        width: 6px;\
                        margin: -8px 0;\
                    }\
                    QSlider::add-page:horizontal {\
                        background: rgb(61,61,61);\
                    }\
                    \
                    QSlider::sub-page:horizontal {\
                        background: #707D9C;\
                    }\
                "));
    pSlider->setValue(defaultMemSize);
    SLIDER_INFO sliderInfo;
    sliderInfo.min = 0;
    sliderInfo.max = defaultMaxMemSize;
    sliderInfo.step = 3;
    pSlider->setSingleStep(sliderInfo.step);
    pSlider->setRange(sliderInfo.min, sliderInfo.max);
    pLayout->addWidget(cacheMemoryLabel, 1, 0, Qt::AlignLeft | Qt::AlignVCenter);
    pLayout->addWidget(cacheMemoryLineEdit, 1, 1, Qt::AlignLeft | Qt::AlignVCenter);
    pLayout->addWidget(pSlider, 1, 2, Qt::AlignVCenter);
    connect(cacheMemoryLineEdit, &ZLineEdit::returnPressed, [pSlider, cacheMemoryLineEdit, this]() {
        pSlider->setRange(0, cacheMemoryLineEdit->text().toInt());
        if (GraphModel* pModel = QVariantPtr<GraphModel>::asPtr(m_nodeIdx.data(QtRole::ROLE_GRAPH)))
        {
            pModel->setData(m_nodeIdx, QtRole::ROLE_DOPNETWORK_MAXMEM, cacheMemoryLineEdit->text().toInt());
        }
    });
    connect(pSlider, &QSlider::valueChanged, [cacheMemoryLineEdit, this](int newVal) {
        cacheMemoryLineEdit->setText(QString::number(newVal));
        if (GraphModel* pModel = QVariantPtr<GraphModel>::asPtr(m_nodeIdx.data(QtRole::ROLE_GRAPH)))
        {
            pModel->setData(m_nodeIdx, QtRole::ROLE_DOPNETWORK_MEM, newVal);
        }
    });

    QCheckBox* allowCacheToDiskCheckBox = new QCheckBox(pWidget);
    allowCacheToDiskCheckBox->setChecked(allowCacheToDisk);
    ZTextLabel* allowCacheToDiskLable = new ZTextLabel("Allow Cache To Disk");
    allowCacheToDiskLable->setFont(font);
    allowCacheToDiskLable->setTextColor(QColor(255, 255, 255, 255 * 0.7));
    allowCacheToDiskLable->setHoverCursor(Qt::ArrowCursor);
    pLayout->addWidget(allowCacheToDiskLable, 2, 0, Qt::AlignLeft | Qt::AlignVCenter);
    pLayout->addWidget(allowCacheToDiskCheckBox, 2, 1, Qt::AlignVCenter);
    connect(allowCacheToDiskCheckBox, &QCheckBox::stateChanged, [this](int state) {
        bool bChecked = (state == Qt::Checked);
        if (GraphModel* pModel = QVariantPtr<GraphModel>::asPtr(m_nodeIdx.data(QtRole::ROLE_GRAPH)))
        {
            pModel->setData(m_nodeIdx, QtRole::ROLE_DOPNETWORK_CACHETODISK, bChecked);
        }
    });

    this->insertTab(1, scrollArea, "cache");
}

zenoDopNetworkPanel::~zenoDopNetworkPanel()
{}

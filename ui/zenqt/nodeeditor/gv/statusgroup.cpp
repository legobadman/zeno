#include "statusgroup.h"
#include "statusbutton.h"
#include "zenosvgitem.h"
#include "style/zenostyle.h"
#include "nodeeditor/gv/zgraphicstextitem.h"


StatusGroup::StatusGroup(bool bHasOptimStatus, RoundRectInfo info, QGraphicsItem* parent)
    : ZLayoutBackground(parent)
    , m_minOptim(nullptr)
    , m_optim(nullptr)
{
    setColors(false, QColor(0, 0, 0, 0));

    RoundRectInfo rectInfo, roundInfo;
    rectInfo.W = info.W;
    rectInfo.H = info.H;
    roundInfo = info;

    m_minMute = new StatusButton(rectInfo);
    m_minMute->setColor(false, QColor("#E7CD2F"), QColor("#2F3135"));

    m_minView = new StatusButton(roundInfo);
    m_minView->setColor(false, QColor("#26C5C5"), QColor("#2F3135"));

    ZGraphicsLayout* pLayout = new ZGraphicsLayout(true);
    pLayout->setSpacing(1);
    if (bHasOptimStatus) {
        m_minOptim = new StatusButton(rectInfo);
        m_minOptim->setColor(false, QColor("#E302F8"), QColor("#2F3135"));
        pLayout->addItem(m_minOptim);
    }
    pLayout->addItem(m_minMute);
    pLayout->addItem(m_minView);
    this->setLayout(pLayout);

    if (bHasOptimStatus) {
        m_optim = new ZenoImageItem(
            ":/icons/ONCE_dark.svg",
            ":/icons/ONCE_light.svg",
            ":/icons/ONCE_light.svg",
            ZenoStyle::dpiScaledSize(QSize(50, 42)),
            this);
        m_optim->setCheckable(true);
        m_optim->hide();
    }

    m_mute = new ZenoImageItem(
        ":/icons/MUTE_dark.svg",
        ":/icons/MUTE_light.svg",
        ":/icons/MUTE_light.svg",
        ZenoStyle::dpiScaledSize(QSize(50, 42)),
        this);
    m_mute->setCheckable(true);
    m_mute->setZValue(1000);
    m_mute->hide();

    m_view = new ZenoImageItem(
        ":/icons/VIEW_dark.svg",
        ":/icons/VIEW_light.svg",
        ":/icons/VIEW_light.svg",
        ZenoStyle::dpiScaledSize(QSize(50, 42)),
        this);
    m_view->setCheckable(true);
    m_view->setZValue(1000);
    m_view->hide();

    QSizeF sz2 = m_mute->size();
    qreal sMarginTwoBar = 0;
    //todo: kill these magin number.

    QPointF base = QPointF(0 + ZenoStyle::dpiScaled(30), -sz2.height() - sMarginTwoBar);
    if (bHasOptimStatus) {
        m_optim->setPos(base);
        base += QPointF(ZenoStyle::dpiScaled(38), 0);
    }
    m_mute->setPos(base);
    base += QPointF(ZenoStyle::dpiScaled(38), 0);
    m_view->setPos(base);

    if (m_minOptim && m_optim) {
        connect(m_minOptim, SIGNAL(hoverChanged(bool)), m_optim, SLOT(setHovered(bool)));
        connect(m_minOptim, SIGNAL(toggled(bool)), m_optim, SLOT(toggle(bool)));
        connect(m_optim, SIGNAL(hoverChanged(bool)), m_minOptim, SLOT(setHovered(bool)));
        connect(m_optim, SIGNAL(toggled(bool)), m_minOptim, SLOT(toggle(bool)));
        connect(m_minOptim, &StatusButton::toggled, [=](bool hovered) {
            emit toggleChanged(STATUS_OWNING, hovered);
            });
    }

    connect(m_minView, SIGNAL(hoverChanged(bool)), m_view, SLOT(setHovered(bool)));
    connect(m_minView, SIGNAL(toggled(bool)), m_view, SLOT(toggle(bool)));
    connect(m_view, SIGNAL(hoverChanged(bool)), m_minView, SLOT(setHovered(bool)));
    connect(m_view, SIGNAL(toggled(bool)), m_minView, SLOT(toggle(bool)));
    connect(m_minView, &StatusButton::toggled, [=](bool hovered) {
        emit toggleChanged(STATUS_VIEW, hovered);
        });

    connect(m_minMute, SIGNAL(hoverChanged(bool)), m_mute, SLOT(setHovered(bool)));
    connect(m_minMute, SIGNAL(toggled(bool)), m_mute, SLOT(toggle(bool)));
    connect(m_mute, SIGNAL(hoverChanged(bool)), m_minMute, SLOT(setHovered(bool)));
    connect(m_mute, SIGNAL(toggled(bool)), m_minMute, SLOT(toggle(bool)));
    connect(m_minMute, &StatusButton::toggled, [=](bool hovered) {
        emit toggleChanged(STATUS_MUTE, hovered);
        });

    setAcceptHoverEvents(true);
}

void StatusGroup::updateRightButtomRadius(bool bHasRadius) {
    m_minView->updateRightButtomRadius(bHasRadius);
}

QRectF StatusGroup::boundingRect() const
{
    return _base::boundingRect();
}

void StatusGroup::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    _base::paint(painter, option, widget);
}

void StatusGroup::setChecked(STATUS_BTN btn, bool bChecked)
{

}

void StatusGroup::setOptions(int options)
{

}

void StatusGroup::setView(bool isView)
{
    m_view->toggle(isView);
}

void StatusGroup::onZoomed()
{
    if (1 - editor_factor > 0.00001f)
    {
        QSize size = QSize(ZenoStyle::scaleWidth(50), ZenoStyle::scaleWidth(42));
        if (m_optim)
            m_optim->resize(size);
        m_mute->resize(size);
        m_view->resize(size);

        QSizeF sz2 = m_mute->size();
        qreal sMarginTwoBar = ZenoStyle::dpiScaled(0);
        QPointF base = QPointF(ZenoStyle::scaleWidth(0), -sz2.height() - sMarginTwoBar);
        qreal offset = ZenoStyle::scaleWidth(38);
        if (m_optim) {
            base += QPointF(offset, 0);
            m_optim->setPos(base);
        }
        base += QPointF(offset, 0);
        m_mute->setPos(base);
        base += QPointF(offset, 0);
        m_view->setPos(base);
    }
}

void StatusGroup::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
    m_mute->show();
    m_view->show();
    if (m_optim)
        m_optim->show();
    _base::hoverEnterEvent(event);
}

void StatusGroup::hoverMoveEvent(QGraphicsSceneHoverEvent* event)
{
    _base::hoverMoveEvent(event);
}

void StatusGroup::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    if (!m_mute->isHovered())
        m_mute->hide();
    if (!m_view->isHovered())
        m_view->hide();
    if (m_optim) {
        if (!m_optim->isHovered())
            m_optim->hide();
    }
    _base::hoverLeaveEvent(event);
}

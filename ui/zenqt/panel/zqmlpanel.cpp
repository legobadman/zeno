#include "zenoapplication.h"
#include "zqmlpanel.h"
#include "model/GraphModel.h"


ZQmlPanel::ZQmlPanel(QWidget* parent)
    : QQuickWidget(zenoApp->getQmlEngine(), parent)
{
    //QQuickStyle::setStyle("Material");
    //QQmlApplicationEngine* engine = zenoApp->getQmlEngine();
    setResizeMode(QQuickWidget::SizeRootObjectToView);
    ////rootContext()->setContextProperty("nodesModel", pGraphM);
    setSource(QUrl(QStringLiteral("qrc:/testQan.qml")));
}

void ZQmlPanel::focusInEvent(QFocusEvent* event)
{
    this->repaint();
    this->quickWindow();
    QQuickWidget::focusInEvent(event);
}

void ZQmlPanel::focusOutEvent(QFocusEvent* event)
{
    QQuickWidget::focusOutEvent(event);
}

void ZQmlPanel::enterEvent(QEvent* event)
{
    QQuickWidget::enterEvent(event);
}

void ZQmlPanel::leaveEvent(QEvent* event)
{
    QQuickWidget::leaveEvent(event);
}



ZQmlGraphWidget::ZQmlGraphWidget(GraphModel* pModel, QWidget* parent)
    : QQuickWidget(zenoApp->getQmlEngine(), parent)
{
    setResizeMode(QQuickWidget::SizeRootObjectToView);
    rootContext()->setContextProperty("nodesModel", pModel);
    setSource(QUrl(QStringLiteral("qrc:/testQan.qml")));
}

void ZQmlGraphWidget::focusInEvent(QFocusEvent* event)
{
    this->repaint();
    QQuickWidget::focusInEvent(event);
}
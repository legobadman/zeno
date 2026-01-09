#pragma once

#ifndef __ZQML_PANEL_H__
#define __ZQML_PANEL_H__

#include <QtQuickWidgets/QQuickWidget>

class GraphModel;

class ZQmlPanel : public QQuickWidget
{
    Q_OBJECT
public:
    ZQmlPanel(QWidget* parent = nullptr);
    void reload();

protected:
    void focusInEvent(QFocusEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;
    void enterEvent(QEvent* event) override;
    void leaveEvent(QEvent* event) override;
};

class ZQmlGraphWidget : public QQuickWidget
{
    Q_OBJECT
public:
    ZQmlGraphWidget(GraphModel* pModel, QWidget* parent = nullptr);

protected:
    void focusInEvent(QFocusEvent* event) override;
};

#endif
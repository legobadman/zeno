#pragma once

#ifndef _ZOPENGL_QUICKVIEW_H__
#define _ZOPENGL_QUICKVIEW_H__

#include <QtWidgets>
#include <QQuickView>

class ZQmlRender;
class Camera;

class ZOpenGLQuickView : public QQuickView
{
    Q_OBJECT
public:
    explicit ZOpenGLQuickView(QWindow* parent = 0);

private:
    void initializeUnderlay();
    void synchronizeUnderlay();
    void renderUnderlay();
    void invalidateUnderlay();

    Camera* m_camera;
    ZQmlRender* m_renderer;
};

class ZOpenGLQuickWidget : public QWidget
{
public:
    explicit ZOpenGLQuickWidget(QWidget* parent = 0);
};

#endif
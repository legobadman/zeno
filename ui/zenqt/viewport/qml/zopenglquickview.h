#pragma once

#ifndef _ZOPENGL_QUICKVIEW_H__
#define _ZOPENGL_QUICKVIEW_H__

#include <QtWidgets>
#include <QQuickView>
#include "zqmlrender.h"

class ZQmlRender;
class Camera;

class ZOpenGLQuickView : public QQuickView
{
    Q_OBJECT
public:
    explicit ZOpenGLQuickView(QWindow* parent = 0);
    void reload_objects(const zeno::render_reload_info& info);

//TODO:
    void cleanUpView() {}
    void testCleanUp() {}
    void cleanUpScene() {}
    Zenovis* getZenoVis() { return nullptr; }
    zenovis::Session* getSession() const { return m_renderer->getSession(); }
    void resizeGL(int nx, int ny);
    void updateGL() { update(); }
    std::shared_ptr<zeno::Picker> picker() const { return nullptr; }
    void updateCameraProp(float aperture, float disPlane) {

    }
    void updatePerspective();
    void setNumSamples(int samples);
    void setCameraRes(const QVector2D& res);
    void setSafeFrames(bool bLock, int nx, int ny);
    bool isCameraMoving() const;
    void setSimpleRenderOption();
    void clearTransformer();
    void changeTransformOperation(const QString& node);
    void changeTransformOperation(int mode);
    void glDrawForCommandLine() {

    }
    void cameraLookTo(zenovis::CameraLookToDir dir);
    void setViewWidgetInfo(DockContentWidgetInfo& info);

    //DEPRECATED:
    void load_objects() {}
    void load_object(zeno::render_update_info info) {}

signals:
    void emitMousePress(QMouseEvent*);
    void emitMouseMove(QMouseEvent*);
    void emitMouseRelease(QMouseEvent*);

protected:
    void resizeEvent(QResizeEvent*) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    void initializeUnderlay();
    void synchronizeUnderlay();
    void renderUnderlay();
    void invalidateUnderlay();

#ifdef BASE_KDAB
    Camera* m_camera;
#endif
    ZQmlRender* m_renderer;
    zeno::render_reload_info m_cache_info;
};

class ZOpenGLQuickWidget : public QWidget
{
public:
    explicit ZOpenGLQuickWidget(QWidget* parent = 0);
};

#endif
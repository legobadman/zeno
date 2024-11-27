#pragma once

#ifndef _ZQML_RENDER_H__
#define _ZQML_RENDER_H__

#include <QObject>
#include <QScopedPointer>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include "../viewport/transform.h"
#include "../viewport/picker.h"
#include <QOpenGLBuffer>
#include <QOpenGLTexture>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <zeno/core/common.h>
#include <zenovis/Session.h>
#include <zenovis/Camera.h>
#include "layout/winlayoutrw.h"

//#define BASE_KDAB

class Zenovis;
class CameraControl;

class ZQmlRender : public QObject
{
    Q_OBJECT
public:
    explicit ZQmlRender(QObject* parent = 0);
    ~ZQmlRender();

    enum CoordinateMirroring {
        DoNotMirrorCoordinates,
        MirrorYCoordinate
    };

    // All assume that the GL context is current.
    void initialize(QOpenGLContext* context, CoordinateMirroring cm = DoNotMirrorCoordinates);
    void render();
    void invalidate();

    void setAzimuth(float azimuth);
    void setElevation(float elevation);
    void setDistance(float distance);
    void resize(int nx, int ny);

    void fakeMousePressEvent(QMouseEvent* event);
    void fakeMouseReleaseEvent(QMouseEvent* event);
    void fakeMouseMoveEvent(QMouseEvent* event);
    void fakeWheelEvent(QWheelEvent* event);

    void updatePerspective();
    void setNumSamples(int samples);
    void setCameraRes(const QVector2D& res);
    zenovis::Session* ZQmlRender::getSession() const;
    void setSafeFrames(bool bLock, int nx, int ny);
    void setSimpleRenderOption();
    void clearTransformer();
    void changeTransformOperation(const QString& node);
    void changeTransformOperation(int mode);
    void cameraLookTo(zenovis::CameraLookToDir dir);
    void setViewWidgetInfo(DockContentWidgetInfo& info);

public slots:
    void reload_objects(const zeno::render_reload_info& info);

private:
#ifdef BASE_KDAB
    QScopedPointer<QOpenGLBuffer> m_positionsBuffer;
    QScopedPointer<QOpenGLBuffer> m_normalsBuffer;
    QScopedPointer<QOpenGLBuffer> m_indicesBuffer;
    QScopedPointer<QOpenGLShaderProgram> m_shaderProgram;
    QScopedPointer<QOpenGLVertexArrayObject> m_vao;

    int m_indicesCount;
    CoordinateMirroring m_coordinateMirroring;
    float m_azimuth;
    float m_elevation;
    float m_distance;
#else
    std::shared_ptr<zeno::Picker> m_picker;
    std::shared_ptr<zeno::FakeTransformer> m_fakeTrans;
    Zenovis* m_zenovis;
    CameraControl* m_camera;
#endif
};


#endif
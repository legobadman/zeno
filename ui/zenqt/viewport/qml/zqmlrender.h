#pragma once

#ifndef _ZQML_RENDER_H__
#define _ZQML_RENDER_H__

#include <QObject>
#include <QScopedPointer>
#include <QOpenGLContext>
#include <QOpenGLFunctions>

#include <QOpenGLBuffer>
#include <QOpenGLTexture>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>

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
    void initialize(CoordinateMirroring cm = DoNotMirrorCoordinates);
    void render();
    void invalidate();

    void setAzimuth(float azimuth);
    void setElevation(float elevation);
    void setDistance(float distance);

private:
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
};


#endif
#pragma once

#ifndef _ZOPENGL_RENDER_H__
#define _ZOPENGL_RENDER_H__

#include <QObject>
#include <QScopedPointer>
#include <memory>
#include <zenovis/Scene.h>

struct IGLDraw {
    virtual void draw() = 0;
};

class RenderEngineGL;

class ZOpenGLRender : public QObject
{
    Q_OBJECT
public:
    explicit ZOpenGLRender(zenovis::Scene* scene_, QObject* parent = 0);
    ~ZOpenGLRender();

    void initialize();
    void render();
    void invalidate();

private:
    QScopedPointer<RenderEngineGL> m_render;
    zenovis::Scene* scene;
};

std::unique_ptr<IGLDraw> makeGLDraw_Axis(zenovis::Scene* scene);
std::unique_ptr<IGLDraw> makeGLDraw_ObjHighlight(zenovis::Scene* scene);
std::unique_ptr<IGLDraw> makeGLDraw_Grid(zenovis::Scene* scene);

#endif
#include "zopenglrender.h"
#include <QOpenGLVertexArrayObject>
#include "globjectsloader.h"
#include "framebufferrender.h"
#include <zenovis/Scene.h>
#include <zenovis/DrawOptions.h>
#include <QOpenGLFunctions>


class RenderEngineGL
{
public:
    RenderEngineGL(zenovis::Scene* scene_)
        : scene(scene_)
        , m_vao(new QOpenGLVertexArrayObject)
        , objsloader(new GLObjectsLoader)
        , fbr(new FrameBufferRender(scene_))
        , primHighlight(makeGLDraw_ObjHighlight(scene))
    {
        hudGraphics.push_back(makeGLDraw_Grid(scene));
        hudGraphics.push_back(makeGLDraw_Axis(scene));
    }

    void reload(const zeno::render_reload_info& info) {
        objsloader->reload(info);
    }

    void draw() {
        QOpenGLFunctions* qgl = QOpenGLContext::currentContext()->functions();
        qgl->glDepthFunc(GL_GREATER);
        qgl->glClearDepthf(0.0);
        qgl->glClearColor(scene->drawOptions->bgcolor.r, scene->drawOptions->bgcolor.g,
            scene->drawOptions->bgcolor.b, 0.0f);
        qgl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    }

private:
    QScopedPointer<QOpenGLVertexArrayObject> m_vao;
    QScopedPointer<GLObjectsLoader> objsloader;
    std::vector<std::unique_ptr<IGLDraw>> hudGraphics;
    QScopedPointer<FrameBufferRender> fbr;
    std::unique_ptr<IGLDraw> primHighlight;
    zenovis::Scene* scene;
    //TODO: highlight
};


ZOpenGLRender::ZOpenGLRender(zenovis::Scene* scene_, QObject* parent)
    : QObject(parent)
    , m_render(new RenderEngineGL(scene_))
    , scene(scene_)
{

}

ZOpenGLRender::~ZOpenGLRender() {

}

void ZOpenGLRender::initialize() {

}

void ZOpenGLRender::render() {

}

void ZOpenGLRender::invalidate() {

}
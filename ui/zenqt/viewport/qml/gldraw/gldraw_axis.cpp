#include "../zopenglrender.h"
#include <zenovis/Scene.h>
#include <QOpenGLBuffer>
#include <QScopedPointer>
#include <QOpenGLContext>
#include <QOpenGLFunctions>


class GLDraw_Axis : public IGLDraw {
public:
    GLDraw_Axis(zenovis::Scene* scene_) : m_scene(scene_) {

    }
    GLDraw_Axis(const GLDraw_Axis&) = delete;
    GLDraw_Axis(GLDraw_Axis&&) = delete;

    void draw() override {

    }

private:
    zenovis::Scene* m_scene;
};

std::unique_ptr<IGLDraw> makeGLDraw_Axis(zenovis::Scene* scene) {
    return std::make_unique<GLDraw_Axis>(scene);
}
#include "../zopenglrender.h"
#include <zenovis/Scene.h>
#include <QOpenGLBuffer>
#include <QScopedPointer>
#include <QOpenGLContext>
#include <QOpenGLFunctions>


class GLDraw_ObjHighlight : public IGLDraw {
public:
    GLDraw_ObjHighlight(zenovis::Scene* scene_) : scene(scene_) {

    }
    GLDraw_ObjHighlight(const GLDraw_ObjHighlight&) = delete;
    GLDraw_ObjHighlight(GLDraw_ObjHighlight&&) = delete;

    void draw() override {

    }
private:
    zenovis::Scene* scene;
};

std::unique_ptr<IGLDraw> makeGLDraw_ObjHighlight(zenovis::Scene* scene) {
    return std::make_unique<GLDraw_ObjHighlight>(scene);
}
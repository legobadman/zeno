#include "../zopenglrender.h"
#include <zenovis/Scene.h>
#include <QOpenGLBuffer>
#include <QScopedPointer>
#include <QOpenGLContext>
#include <QOpenGLFunctions>


class GLDraw_Grid : public IGLDraw {
public:
    GLDraw_Grid(zenovis::Scene* scene_) : scene(scene_) {

    }
    GLDraw_Grid(const GLDraw_Grid&) = delete;
    GLDraw_Grid(GLDraw_Grid&&) = delete;

    void draw() override {

    }

private:
    zenovis::Scene* scene;
};

std::unique_ptr<IGLDraw> makeGLDraw_Grid(zenovis::Scene* scene) {
    return std::make_unique<GLDraw_Grid>(scene);
}
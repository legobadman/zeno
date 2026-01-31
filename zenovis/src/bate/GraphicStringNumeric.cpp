#include <zenovis/bate/IGraphic.h>
#include <zeno/types/ListObject.h>
#include <zeno/utils/log.h>

namespace zenovis {
    namespace {
        struct GraphicDummy final : IGraphic {
            Scene* scene;

            explicit GraphicDummy(Scene* scene_, zeno::DummyObject* lit) : scene(scene_) {
            }
        };
    }

    void MakeGraphicVisitor::visit(zeno::DummyObject* obj) {
        this->out_result = std::make_unique<GraphicDummy>(this->in_scene, obj);
    }

namespace {

struct GraphicList final : IGraphic {
    Scene *scene;

    explicit GraphicList(Scene *scene_, zeno::ListObject *lst) : scene(scene_) {
        zeno::log_info("ToView got ListObject with size: {}", lst->size());
    }
};

}

void MakeGraphicVisitor::visit(zeno::ListObject *obj) {
     this->out_result = std::make_unique<GraphicList>(this->in_scene, obj);
}

}

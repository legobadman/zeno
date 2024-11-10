#pragma once

#include <zenovis/Camera.h>
#include <zenovis/Scene.h>
#include <type_traits>
#include <functional>
#include <string>
#include <memory>
#include <map>
#include <zeno/core/ObjectManager.h>

namespace zenovis {

struct Scene;

struct RenderEngine {
    virtual void draw(bool record) = 0;
    virtual void update() = 0;
    virtual void cleanupScene() = 0;
    virtual void cleanupAssets() = 0;
    virtual void cleanupWhenExit() = 0;
    virtual void load_objects(const zeno::RenderObjsInfo& objs) {}
    virtual void load_object(zeno::render_update_info info) {}
    virtual ~RenderEngine() = default;
    virtual std::optional<glm::vec3> getClickedPos(int x, int y) { return {}; }
};

class RenderManager {
    static std::map<std::string, std::function<std::unique_ptr<RenderEngine>(Scene *)>> factories;
    std::map<std::string, std::unique_ptr<RenderEngine>> instances;
    std::string defaultEngineName;
    Scene *scene;

public:
    explicit RenderManager(Scene *scene_) : scene(scene_) {
    }

    template <class T, class = std::enable_if_t<std::is_base_of_v<RenderEngine, T>>>
    static int registerRenderEngine(std::string const &name) {
        factories.emplace(name, [] (Scene *s) { return std::make_unique<T>(s); });
        return 1;
    } 

    RenderEngine *getEngine(std::string const &name) {
        auto it = instances.find(name);
        if (it == instances.end()) {
            it = instances.emplace(name, factories.at(name)(scene)).first;
        }
        return it->second.get();
    }

    RenderEngine *getEngine() {
        return getEngine(defaultEngineName);
    }

    std::string getDefaultEngineName() const {
        return defaultEngineName;
    }

    void switchDefaultEngine(std::string const &name) {
        defaultEngineName = name;
    }
};

}

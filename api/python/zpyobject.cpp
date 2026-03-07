#include "zpyobject.h"
#include "apiutil.h"
#include <zeno/core/Graph.h>
#include <zeno/types/UserData.h>
#include <zeno/utils/interfaceutil.h>
#include "zeno_types/reflect/reflection.generated.hpp"


#define THROW_WHEN_CORE_DESTROYED(wpObj) \
auto spNode = wpObj;\
if (!spNode) {\
    throw std::runtime_error("the node has been destroyed in core data");\
}

Zpy_Object::Zpy_Object(zeno::zany2&& obj) : m_wpObject(std::move(obj)) {

}

VAR_USER_DATA Zpy_Object::get_user_data(const std::string& key) {
    auto spObject = m_wpObject.get();
    if (!spObject) {
        throw std::runtime_error("object has been destroyed");
    }
    auto ud = m_wpObject->userData();
    if (ud->has_int(key.c_str())) {
        int v = ud->get_int(key.c_str(), 0);
        return v;
    }
    else if (ud->has_float(key.c_str())) {
        float v = ud->get_float(key.c_str(), 0.0f);
        return v;
    }
    else if (ud->has_vec2i(key.c_str())) {
        zeno::Vec2i v = ud->get_vec2i(key.c_str());
        return zeno::toVec2i(v);
    }
    else if (ud->has_vec3i(key.c_str())) {
        zeno::Vec3i v = ud->get_vec3i(key.c_str());
        return zeno::toVec3i(v);
    }
    else if (ud->has_vec4i(key.c_str())) {
        zeno::Vec4i v = ud->get_vec4i(key.c_str());
        return zeno::toVec4i(v);
    }
    else if (ud->has_vec2f(key.c_str())) {
        zeno::Vec2f v = ud->get_vec2f(key.c_str());
        return zeno::toVec2f(v);
    }
    else if (ud->has_vec3f(key.c_str())) {
        zeno::Vec3f v = ud->get_vec3f(key.c_str());
        return zeno::toVec3f(v);
    }
    else if (ud->has_vec4f(key.c_str())) {
        zeno::Vec4f v = ud->get_vec4f(key.c_str());
        return zeno::toVec4f(v);
    }
    else if (ud->has_string(key.c_str())) {
        char sbuf[512] = {};
        ud->get_string(key.c_str(), "", sbuf, sizeof(sbuf));
        return std::string(sbuf);
    }
    else if (ud->has_bool(key.c_str())) {
        bool v = ud->get_bool(key.c_str(), false);
        return v ? 1 : 0;
    }
    else {
        throw std::runtime_error("GetUserData3: unsupported UserData type for given key");
    }
}

void Zpy_Object::set_user_data(const std::string& key, const VAR_USER_DATA& dat) {
    auto spObject = m_wpObject.get();
    if (!spObject) {
        throw std::runtime_error("object has been destroyed");
    }

    auto csKey = key.c_str();
    auto ud = m_wpObject->userData();

    std::visit([&](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, int>) {
            ud->set_int(csKey, arg);
        }
        else if constexpr (std::is_same_v<T, float>) {
            ud->set_float(csKey, arg);
        }
        else if constexpr (std::is_same_v<T, std::string>) {
            ud->set_string(csKey, arg.c_str());
        }
        else if constexpr (std::is_same_v<T, zeno::vec2i>) {
            ud->set_vec2i(csKey, zeno::toAbiVec2i(arg));
        }
        else if constexpr (std::is_same_v<T, zeno::vec2f>) {
            ud->set_vec2f(csKey, zeno::toAbiVec2f(arg));
        }
        else if constexpr (std::is_same_v<T, zeno::vec3f>) {
            ud->set_vec3f(csKey, zeno::toAbiVec3f(arg));
        }
        else if constexpr (std::is_same_v<T, zeno::vec3i>) {
            ud->set_vec3i(csKey, zeno::toAbiVec3i(arg));
        }
        else if constexpr (std::is_same_v<T, zeno::vec4i>) {
            ud->set_vec4i(csKey, zeno::toAbiVec4i(arg));
        }
        else if constexpr (std::is_same_v<T, zeno::vec4f>) {
            ud->set_vec4f(csKey, zeno::toAbiVec4f(arg));
        }
        }, dat);
}

std::vector<Zpy_Object> Zpy_Object::toList() const {
    auto spObject = m_wpObject.get();
    if (!spObject) {
        throw std::runtime_error("object has been destroyed");
    }
    if (auto spList = dynamic_cast<zeno::ListObject*>(spObject)) {
        std::vector<Zpy_Object> vec;
        for (auto spObj : spList->get()) {
            vec.push_back(Zpy_Object(zeno::zany2(spObj->clone())));
        }
        return vec;
    }
    else {
        throw std::runtime_error("the object is not a list object");
    }
}


Zpy_Camera::Zpy_Camera(
    py::list pos,
    py::list up,
    py::list view,
    float fov,
    float aperture,
    float focalPlaneDistance
)
{
    //先默认在mainGraph里创建，避免还要指定一个graph这种麻烦（而且Camera这种大概率只要在main创建）
    auto spNode = zeno::getSession().mainGraph()->createNode("MakeCamera");
    if (!spNode) {
        throw std::runtime_error("cannot create camera because of internal error");
    }
    m_wpNode = spNode;

    zeno::vecvar _pos = zpyapi::pylist2vec(pos);
    if (_pos.size() != 3) { throw std::runtime_error("error dims of `pos`,which should be 3."); }

    zeno::vecvar _up = zpyapi::pylist2vec(up);
    if (_up.size() != 3) { throw std::runtime_error("error dims of `up`,which should be 3."); }

    zeno::vecvar _view = zpyapi::pylist2vec(view);
    if (_view.size() != 3) { throw std::runtime_error("error dims of `view`,which should be 3."); }

    //其他参数暂时不考虑公式的情况
    spNode->getNodeParams().update_param("pos", _pos);
    spNode->getNodeParams().update_param("up", _up);
    spNode->getNodeParams().update_param("view", _view);
    spNode->getNodeParams().update_param("fov", fov);
    spNode->getNodeParams().update_param("aperture", aperture);
    spNode->getNodeParams().update_param("focalPlaneDistance", focalPlaneDistance);
    spNode->getNodeStatus().set_view(true);

    auto nodename = spNode->get_name();
    zeno::render_reload_info render_;
    zeno::getSession().mainGraph()->applyNodes({ nodename }, render_);
}

Zpy_Camera::Zpy_Camera(zeno::ZNode* wpNode)
    : m_wpNode(wpNode)
{
}

void Zpy_Camera::run() {
    THROW_WHEN_CORE_DESTROYED(m_wpNode)
    auto nodename = spNode->get_name();
    zeno::render_reload_info render_;
    zeno::getSession().mainGraph()->applyNodes({ nodename }, render_);
}

std::unique_ptr<zeno::CameraObject> Zpy_Camera::getCamera() const {
    THROW_WHEN_CORE_DESTROYED(m_wpNode)
    zeno::IObject2* spResObj = spNode->getNodeParams().get_default_output_object()->clone();
    return std::unique_ptr<zeno::CameraObject>(static_cast<zeno::CameraObject*>(spResObj));
}

py::list Zpy_Camera::getPos() const {
    THROW_WHEN_CORE_DESTROYED(m_wpNode)
    auto camera = getCamera();
    if (!camera) throw std::runtime_error("the camera object cannot be created.");
    return zpyapi::vec2pylist(camera->pos);
}

void Zpy_Camera::setPos(py::list v) {
    THROW_WHEN_CORE_DESTROYED(m_wpNode)
    spNode->getNodeParams().update_param("pos", zpyapi::pylist2vec(v));
    run();
}

py::list Zpy_Camera::getUp() const {
    THROW_WHEN_CORE_DESTROYED(m_wpNode)
    auto camera = getCamera();
    if (!camera) throw std::runtime_error("the camera object cannot be created.");
    return zpyapi::vec2pylist(camera->up);
}

void Zpy_Camera::setUp(py::list v) {
    THROW_WHEN_CORE_DESTROYED(m_wpNode)
    spNode->getNodeParams().update_param("up", zpyapi::pylist2vec(v));
    run();
}

py::list Zpy_Camera::getView() const {
    THROW_WHEN_CORE_DESTROYED(m_wpNode)
    auto camera = getCamera();
    if (!camera) throw std::runtime_error("the camera object cannot be created.");
    return zpyapi::vec2pylist(camera->view);
}

void Zpy_Camera::setView(py::list v) {
    THROW_WHEN_CORE_DESTROYED(m_wpNode)
    spNode->getNodeParams().update_param("view", zpyapi::pylist2vec(v));
    run();
}

float Zpy_Camera::getNear() const {
    THROW_WHEN_CORE_DESTROYED(m_wpNode)
    auto camera = getCamera();
    if (!camera) throw std::runtime_error("the camera object cannot be created.");
    return camera->fnear;
}

void Zpy_Camera::setNear(float near) {
    THROW_WHEN_CORE_DESTROYED(m_wpNode)
    spNode->getNodeParams().update_param("near", near);
    run();
}

float Zpy_Camera::getFar() const {
    THROW_WHEN_CORE_DESTROYED(m_wpNode)
    auto camera = getCamera();
    if (!camera) throw std::runtime_error("the camera object cannot be created.");
    return camera->ffar;
}

void Zpy_Camera::setFar(float far) {
    THROW_WHEN_CORE_DESTROYED(m_wpNode)
    spNode->getNodeParams().update_param("far", far);
    run();
}

float Zpy_Camera::getFov() const {
    THROW_WHEN_CORE_DESTROYED(m_wpNode)
    auto camera = getCamera();
    if (!camera) throw std::runtime_error("the camera object cannot be created.");
    return camera->fov;
}

void Zpy_Camera::setFov(float fov) {
    THROW_WHEN_CORE_DESTROYED(m_wpNode)
    spNode->getNodeParams().update_param("fov", fov);
    run();
}

float Zpy_Camera::getAperture() const {
    THROW_WHEN_CORE_DESTROYED(m_wpNode)
    auto camera = getCamera();
    if (!camera) throw std::runtime_error("the camera object cannot be created.");
    return camera->aperture;
}

void Zpy_Camera::setAperture(float aperture) {
    THROW_WHEN_CORE_DESTROYED(m_wpNode)
    spNode->getNodeParams().update_param("aperture", aperture);
    run();
}

float Zpy_Camera::getFocalPlaneDistance() const {
    THROW_WHEN_CORE_DESTROYED(m_wpNode)
    auto camera = getCamera();
    if (!camera) throw std::runtime_error("the camera object cannot be created.");
    return camera->focalPlaneDistance;
}

void Zpy_Camera::setFocalPlaneDistance(float focal) {
    THROW_WHEN_CORE_DESTROYED(m_wpNode)
    spNode->getNodeParams().update_param("focalPlaneDistance", focal);
    run();
}


Zpy_Light::Zpy_Light(
    py::list pos,
    py::list scale,
    py::list rotate,
    py::list color,
    float intensity
)
{
    //先默认在mainGraph里创建，避免还要指定一个graph这种麻烦（而且Camera这种大概率只要在main创建）
    auto spNode = zeno::getSession().mainGraph()->createNode("LightNode");
    if (!spNode) {
        throw std::runtime_error("cannot create light because of internal error");
    }
    m_wpNode = spNode;

    zeno::vecvar _pos = zpyapi::pylist2vec(pos);
    if (_pos.size() != 3) { throw std::runtime_error("error dims of `pos`, which should be 3."); }

    zeno::vecvar _scale = zpyapi::pylist2vec(scale);
    if (_scale.size() != 3) { throw std::runtime_error("error dims of `scale`, which should be 3."); }

    zeno::vecvar _rotate = zpyapi::pylist2vec(rotate);
    if (_rotate.size() != 3) { throw std::runtime_error("error dims of `rotate`, which should be 3."); }

    zeno::vecvar _color = zpyapi::pylist2vec(color);
    if (_color.size() != 3) { throw std::runtime_error("error dims of `color`, which should be 3."); }

    //其他参数暂时不考虑公式的情况
    spNode->getNodeParams().update_param("position", _pos);
    spNode->getNodeParams().update_param("scale", _scale);
    spNode->getNodeParams().update_param("rotate", _rotate);
    spNode->getNodeParams().update_param("color", _color);
    spNode->getNodeParams().update_param("intensity", intensity);
    spNode->getNodeStatus().set_view(true);

    auto nodename = spNode->get_name();
    zeno::render_reload_info render_;
    zeno::getSession().mainGraph()->applyNodes({ nodename }, render_);
}

Zpy_Light::Zpy_Light(zeno::ZNode* wpNode)
    : m_wpNode(wpNode)
{
}

py::list Zpy_Light::getPos() const {
    auto light = getLight();
    if (!light) throw std::runtime_error("the light object cannot be created.");
    auto ud = light->userData();
    zeno::vec3f v = zeno::toVec3f(ud->get_vec3f("pos"));
    return zpyapi::vec2pylist(v);
}

void Zpy_Light::setPos(py::list v) {
    THROW_WHEN_CORE_DESTROYED(m_wpNode)
    spNode->getNodeParams().update_param("position", zpyapi::pylist2vec(v));
    run();
}

py::list Zpy_Light::getScale() const {
    auto light = getLight();
    if (!light) throw std::runtime_error("the light object cannot be created.");
    auto ud = light->userData();
    zeno::vec3f v = zeno::toVec3f(ud->get_vec3f("scale"));
    return zpyapi::vec2pylist(v);
}

void Zpy_Light::setScale(py::list v) {
    THROW_WHEN_CORE_DESTROYED(m_wpNode)
    spNode->getNodeParams().update_param("scale", zpyapi::pylist2vec(v));
    run();
}

py::list Zpy_Light::getRotate() const {
    auto light = getLight();
    if (!light) throw std::runtime_error("the light object cannot be created.");
    auto ud = light->userData();
    zeno::vec3f v = zeno::toVec3f(ud->get_vec3f("rotate"));
    return zpyapi::vec2pylist(v);
}

void Zpy_Light::setRotate(py::list v) {
    THROW_WHEN_CORE_DESTROYED(m_wpNode)
    spNode->getNodeParams().update_param("rotate", zpyapi::pylist2vec(v));
    run();
}

py::list Zpy_Light::getColor() const {
    auto light = getLight();
    if (!light) throw std::runtime_error("the light object cannot be created.");
    auto ud = light->userData();
    zeno::vec3f v = zeno::toVec3f(ud->get_vec3f("color"));
    return zpyapi::vec2pylist(v);
}

void Zpy_Light::setColor(py::list v) {
    THROW_WHEN_CORE_DESTROYED(m_wpNode)
    spNode->getNodeParams().update_param("color", zpyapi::pylist2vec(v));
    run();
}

float Zpy_Light::getIntensity() const {
    auto light = getLight();
    if (!light) throw std::runtime_error("the light object cannot be created.");
    auto ud = light->userData();
    return ud->get_float("intensity");
}

void Zpy_Light::setIntensity(float intensity) {
    THROW_WHEN_CORE_DESTROYED(m_wpNode)
    spNode->getNodeParams().update_param("intensity", intensity);
    run();
}

void Zpy_Light::run() {
    THROW_WHEN_CORE_DESTROYED(m_wpNode)
    auto nodename = spNode->get_name();
    zeno::render_reload_info render_;
    zeno::getSession().mainGraph()->applyNodes({ nodename }, render_);
}

std::shared_ptr<zeno::PrimitiveObject> Zpy_Light::getLight() const {
    THROW_WHEN_CORE_DESTROYED(m_wpNode)

    auto pObject = spNode->getNodeParams().get_default_output_object();
    if (pObject) {
        zeno::IObject2* pResObj = pObject->clone();
        return std::shared_ptr<zeno::PrimitiveObject>(static_cast<zeno::PrimitiveObject*>(pObject));
    }
    return nullptr;
}

#include <zenovis/RenderEngine.h>
#include "cameracontrol.h"
#include "zenovis.h"
//#include <zenovis/Camera.h>
#include <zenovis/ObjectsManager.h>
#include "zenomainwindow.h"
#include "nodeeditor/gv/zenographseditor.h"
#include <zeno/types/UserData.h>
#include "settings/zenosettingsmanager.h"
#include "glm/gtx/quaternion.hpp"
#include "zeno/core/Session.h"
#include <cmath>


using std::string;
using std::unordered_set;
using std::unordered_map;

CameraControl::CameraControl(
            Zenovis* pZenovis,
            std::shared_ptr<zeno::FakeTransformer> transformer,
            std::shared_ptr<zeno::Picker> picker,
            QObject* parent)
    : QObject(parent)
    , m_zenovis(pZenovis)
    , m_transformer(transformer)
    , m_picker(picker)
    , m_res(1, 1)
{
    updatePerspective();
}

void CameraControl::setRes(QVector2D res) {
    m_res = res;
}

glm::vec3 CameraControl::getPos() const {
    auto *scene = m_zenovis->getSession()->get_scene();
    return scene->camera->getPos();
}
void CameraControl::setPos(glm::vec3 value) {
    auto *scene = m_zenovis->getSession()->get_scene();
    scene->camera->setPos(value);
}
glm::vec3 CameraControl::getPivot() const {
    auto *scene = m_zenovis->getSession()->get_scene();
    return scene->camera->getPivot();
}
void CameraControl::setPivot(glm::vec3 value) {
    auto *scene = m_zenovis->getSession()->get_scene();
    scene->camera->setPivot(value);
}
glm::quat CameraControl::getRotation() {
    auto *scene = m_zenovis->getSession()->get_scene();
    return scene->camera->m_rotation;
}
void CameraControl::setRotation(glm::quat value) {
    auto *scene = m_zenovis->getSession()->get_scene();
    scene->camera->m_rotation = value;
}
bool CameraControl::getOrthoMode() const {
    auto *scene = m_zenovis->getSession()->get_scene();
    return scene->camera->m_ortho_mode;
}
void CameraControl::setOrthoMode(bool orthoMode) {
    auto *scene = m_zenovis->getSession()->get_scene();
    scene->camera->m_ortho_mode = orthoMode;
}
float CameraControl::getRadius() const {
    auto *scene = m_zenovis->getSession()->get_scene();
    return scene->camera->get_radius();
}

float CameraControl::getFOV() const {
    auto *scene = m_zenovis->getSession()->get_scene();
    return scene->camera->m_fov;
}
void CameraControl::setFOV(float fov) {
    auto *scene = m_zenovis->getSession()->get_scene();
    scene->camera->m_fov = fov;
}

float CameraControl::getAperture() const {
    auto *scene = m_zenovis->getSession()->get_scene();
    return scene->camera->m_aperture;
}

void CameraControl::setAperture(float aperture) {
    auto *scene = m_zenovis->getSession()->get_scene();
    scene->camera->m_aperture = aperture;
}
float CameraControl::getDisPlane() const {
    auto *scene = m_zenovis->getSession()->get_scene();
    return scene->camera->focalPlaneDistance;
}
void CameraControl::setDisPlane(float disPlane) {
    auto *scene = m_zenovis->getSession()->get_scene();
    scene->camera->focalPlaneDistance = disPlane;
}

void CameraControl::fakeMousePressEvent(ViewMouseInfo info)
{
    ZASSERT_EXIT(m_zenovis);
    auto scene = m_zenovis->getSession()->get_scene();
    const qreal x = info.pos.x(), y = info.pos.y();
    if (info.buttons == Qt::MiddleButton) {
        middle_button_pressed = true;
        if (zeno::getSession().userData().get2<bool>("viewport-depth-aware-navigation", true)) {
            m_hit_posWS = scene->renderMan->getEngine()->getClickedPos(x, y);
            if (m_hit_posWS.has_value()) {
                scene->camera->setPivot(m_hit_posWS.value());
            }
        }
    }
    auto m_picker = this->m_picker.lock();
    auto m_transformer = this->m_transformer.lock();
    int button = Qt::NoButton;
    ZenoSettingsManager& settings = ZenoSettingsManager::GetInstance();
    settings.getViewShortCut(ShortCut_MovingView, button);
    settings.getViewShortCut(ShortCut_RotatingView, button);
    bool bTransform = false;
    auto front = scene->camera->get_lodfront();
    auto dir = screenPosToRayWS(x / res().x(), y / res().y());
    if (m_transformer)
    {
        if (info.buttons & Qt::LeftButton && !scene->selected.empty() && m_transformer->isTransformMode() &&
            m_transformer->clickedAnyHandler(getPos(), dir, front))
        {
            bTransform = true;
        }
    }
    if (!bTransform && (info.buttons & button)) {
        m_lastMidButtonPos = info.pos;
    } else if (info.buttons & Qt::LeftButton) {
        m_boundRectStartPos = info.pos.toPoint();
        // check if clicked a selected object
        if (bTransform)
        {
            m_transformer->startTransform();
        }
    }
}

void CameraControl::lookTo(zenovis::CameraLookToDir dir) {
    ZASSERT_EXIT(m_zenovis);

    switch (dir) {
    case zenovis::CameraLookToDir::front_view:
        break;
    case zenovis::CameraLookToDir::right_view:
        break;
    case zenovis::CameraLookToDir::top_view:
        break;
    case zenovis::CameraLookToDir::back_view:
        break;
    case zenovis::CameraLookToDir::left_view:
        break;
    case zenovis::CameraLookToDir::bottom_view:
        break;
    case zenovis::CameraLookToDir::back_to_origin:
        break;
    default: break;
    }
    setOrthoMode(true);
    updatePerspective();
    zenoApp->getMainWindow()->updateViewport();
}

void CameraControl::clearTransformer() {
    auto m_transformer = this->m_transformer.lock();
    if (!m_transformer)
        return;
    m_transformer->clear();
}

void CameraControl::changeTransformOperation(const QString &node)
{
    auto m_transformer = this->m_transformer.lock();
    if (!m_transformer)
        return;

    auto opt = m_transformer->getTransOpt();
    m_transformer->clear();

    ZASSERT_EXIT(m_zenovis);

    auto scene = m_zenovis->getSession()->get_scene();
    for (auto const &[key, _] : scene->objectsMan->pairs()) {
        if (key.find(node.toStdString()) != std::string::npos) {
            scene->selected.insert(key);
            m_transformer->addObject(key);
        }
    }
    m_transformer->setTransOpt(opt);
    m_transformer->changeTransOpt();
    zenoApp->getMainWindow()->updateViewport();
}

void CameraControl::changeTransformOperation(int mode)
{
    auto m_transformer = this->m_transformer.lock();
    if (!m_transformer)
        return;

    switch (mode) {
    case 0: m_transformer->toTranslate(); break;
    case 1: m_transformer->toRotate(); break;
    case 2: m_transformer->toScale(); break;
    default: break;
    }
    zenoApp->getMainWindow()->updateViewport();
}

void CameraControl::changeTransformCoordSys()
{
    auto m_transformer = this->m_transformer.lock();
    if (!m_transformer)
        return;
    m_transformer->changeCoordSys();
    zenoApp->getMainWindow()->updateViewport();
}

void CameraControl::resizeTransformHandler(int dir)
{
    auto m_transformer = this->m_transformer.lock();
    if (!m_transformer)
        return;
    m_transformer->resizeHandler(dir);
    zenoApp->getMainWindow()->updateViewport();
}

void CameraControl::fakeMouseMoveEvent(ViewMouseInfo info)
{
    auto m_transformer = this->m_transformer.lock();
    bool ctrl_pressed = info.modifiers & Qt::ControlModifier;
    bool alt_pressed = info.modifiers & Qt::AltModifier;

    auto session = m_zenovis->getSession();
    auto scene = session->get_scene();
    float x = info.pos.x(), y = info.pos.y();

    int moveButton = Qt::NoButton;
    int rotateButton = Qt::NoButton;
    ZenoSettingsManager& settings = ZenoSettingsManager::GetInstance();
    int moveKey = settings.getViewShortCut(ShortCut_MovingView, moveButton);
    int rotateKey = settings.getViewShortCut(ShortCut_RotatingView, rotateButton);

    bool bTransform = false;
    if (m_transformer) {
        bTransform = m_transformer->isTransforming();
        // check if hover a handler
        auto front = scene->camera->get_lodfront();
        auto dir = screenPosToRayWS(x / res().x(), y / res().y());
        if (!scene->selected.empty() && !(info.buttons & Qt::LeftButton)) {
            m_transformer->hoveredAnyHandler(getPos(), dir, front);
        }
    }

    if (!bTransform && alt_pressed && (info.buttons & Qt::MiddleButton)) {
        // zoom
        if (zeno::getSession().userData().get2<bool>("viewport-FPN-navigation", false) == false) {
            float dy = y - m_lastMidButtonPos.y();
            auto step = 0.99f;
            float scale = glm::pow(step, -dy);
            auto pos = getPos();
            auto pivot = getPivot();
            auto new_pos = (pos - pivot) * scale + pivot;
            setPos(new_pos);
        }
        m_lastMidButtonPos = QPointF(x, y);
    }
    else if (!bTransform && ctrl_pressed && (info.buttons & Qt::MiddleButton)) {
        // rot roll
        float step = 1.0f;
        float ratio = 1.0;// QApplication::desktop()->devicePixelRatio();
        float dy = y - m_lastMidButtonPos.y();
        dy *= ratio / m_res[1] * step;
        {
            auto rot = getRotation();
            rot = rot * glm::angleAxis(dy, glm::vec3(0, 0, 1));
            setRotation(rot);
        }
        m_lastMidButtonPos = QPointF(x, y);
    }
    else if (!bTransform && (info.buttons & (rotateButton | moveButton))) {
        float ratio = 1.0;// QApplication::desktop()->devicePixelRatio();
        float dx = x - m_lastMidButtonPos.x(), dy = y - m_lastMidButtonPos.y();
        dx *= ratio / m_res[0];
        dy *= ratio / m_res[1];
        //bool shift_pressed = info.modifiers & Qt::ShiftModifier;
        Qt::KeyboardModifiers modifiers = info.modifiers;
        if ((moveKey == modifiers) && (info.buttons & moveButton)) {
            // translate
            if (m_hit_posWS.has_value()) {
                auto ray = screenPosToRayWS(x / res().x(), y / res().y());
                auto new_pos = intersectRayPlane(m_hit_posWS.value(), ray * (-1.0f), getPos(), getViewDir());
                if (new_pos.has_value()) {
                    auto diff = new_pos.value() - getPos();
                    setPivot(getPivot() + diff);
                    setPos(new_pos.value());
                }
            }
            else {
                auto left = getRightDir() * -1.0f;
                auto up = getUpDir();
                auto delta = left * dx + up * dy;
                if (getOrthoMode()) {
                    delta = (left * dx * float(m_res[0]) / float(m_res[1]) + up * dy) * 2.0f;
                }
                auto diff = delta * getRadius();
                setPivot(getPivot() + diff);
                auto new_pos = getPos() + diff;
                setPos(new_pos);
            }
        } else if ((rotateKey == modifiers) && (info.buttons & rotateButton)) {
            float step = 4.0f;
            dx *= step;
            if (getUpDir().y < 0) {
                dx *= -1;
            }
            dy *= step;
            // rot yaw pitch
            setOrthoMode(false);
            {
                auto rot = getRotation();
                auto beforeMat = glm::toMat3(rot);
                rot = glm::angleAxis(-dx, glm::vec3(0, 1, 0)) * rot;
                rot = rot * glm::angleAxis(-dy, glm::vec3(1, 0, 0));
                setRotation(rot);
                auto afterMat = glm::toMat3(rot);
                if (zeno::getSession().userData().get2<bool>("viewport-FPN-navigation", false)) {
                    if (glm::abs(glm::dot(getRightDir(), {0, 1, 0})) > 0.01) {
                        auto right_dir = glm::cross(getViewDir(), {0, 1, 0});
                        auto up_dir = glm::cross(right_dir, getViewDir());
                        glm::mat3 rotation;
                        rotation[0] = right_dir;
                        rotation[1] = up_dir;
                        rotation[2] = -getViewDir();
                        setRotation(glm::quat_cast(rotation));
                    };
                    setPivot(getPos());
                }
                else {
                    auto pos = getPos();
                    auto pivot = getPivot();
                    auto new_pos = afterMat * glm::inverse(beforeMat) * (pos - pivot) + pivot;
                    setPos(new_pos);
                }
            }
        }
        m_lastMidButtonPos = QPointF(x, y);
    } else if (info.buttons & Qt::LeftButton) {
        if (m_transformer)
        {
            if (m_transformer->isTransforming()) {
                auto dir = screenPosToRayWS(x / res().x(), y / res().y());

                // mouse pos
                auto mouse_pos = glm::vec2(x, y);
                mouse_pos[0] = (2 * mouse_pos[0] / res().x()) - 1;
                mouse_pos[1] = 1 - (2 * mouse_pos[1] / res().y());
                // mouse start
                auto mouse_start = glm::vec2(m_boundRectStartPos.x(), m_boundRectStartPos.y());
                mouse_start[0] = (2 * mouse_start[0] / res().x()) - 1;
                mouse_start[1] = 1 - (2 * mouse_start[1] / res().y());

                auto vp = scene->camera->get_proj_matrix() * scene->camera->get_view_matrix();
                m_transformer->transform(getPos(), dir, mouse_start, mouse_pos, scene->camera->get_lodfront(), vp);
                zenoApp->getMainWindow()->updateViewport();
            } else {
                float min_x = std::min((float)m_boundRectStartPos.x(), x) / m_res.x();
                float max_x = std::max((float)m_boundRectStartPos.x(), x) / m_res.x();
                float min_y = std::min((float)m_boundRectStartPos.y(), y) / m_res.y();
                float max_y = std::max((float)m_boundRectStartPos.y(), y) / m_res.y();
                scene->select_box = zeno::vec4f(min_x, min_y, max_x, max_y);
            }
        }
    }
    updatePerspective();
}

void CameraControl::updatePerspective() {
    auto *session = m_zenovis->getSession();
    if (session == nullptr) {
        return;
    }
    m_zenovis->updatePerspective(m_res);
}

void CameraControl::fakeWheelEvent(ViewMouseInfo info) {
    int dy = 0;
    int x = info.pos.x(), y = info.pos.y();
    if (info.modifiers & Qt::AltModifier)
        dy = info.angleDelta.x();
    else
        dy = info.angleDelta.y();
    float scale = (dy >= 0) ? 0.89 : 1 / 0.89;
    bool shift_pressed = (info.modifiers & Qt::ShiftModifier) && !(info.modifiers & Qt::ControlModifier);
    bool aperture_pressed = (info.modifiers & Qt::ControlModifier) && !(info.modifiers & Qt::ShiftModifier);
    bool focalPlaneDistance_pressed =
        (info.modifiers & Qt::ControlModifier) && (info.modifiers & Qt::ShiftModifier);
    float delta = dy > 0 ? 1 : -1;
    int button = Qt::NoButton;
    ZenoSettingsManager& settings = ZenoSettingsManager::GetInstance();
    int scaleKey = settings.getViewShortCut(ShortCut_ScalingView, button);
    if (shift_pressed) {
        auto& inst = ZenoSettingsManager::GetInstance();
        QVariant varEnableShiftChangeFOV = inst.getValue(zsEnableShiftChangeFOV);
        bool bEnableShiftChangeFOV = varEnableShiftChangeFOV.isValid() ? varEnableShiftChangeFOV.toBool() : true;
        if (bEnableShiftChangeFOV) {
            float temp = getFOV() / scale;
            setFOV(temp < 170 ? temp : 170);
        }

    } else if (aperture_pressed) {
        float temp = getAperture() + delta * 0.1;
        setAperture(temp >= 0 ? temp : 0);

    } else if (focalPlaneDistance_pressed) {
        float temp = getDisPlane() + delta * 0.05;
        setDisPlane(temp >= 0.05 ? temp : 0.05);
    } else if (scaleKey == 0 || info.modifiers & scaleKey){
        if (zeno::getSession().userData().get2<bool>("viewport-FPN-navigation", false)) {
            auto FPN_move_speed = zeno::getSession().userData().get2<int>("viewport-FPN-move-speed", 0);
            FPN_move_speed += dy > 0? 1: -1;
            zeno::getSession().userData().set2("viewport-FPN-move-speed", FPN_move_speed);
            auto pMainWindow = zenoApp->getMainWindow();
            if (pMainWindow) {
                pMainWindow->statusbarShowMessage(zeno::format("First Person Navigation: movement speed level: {}", FPN_move_speed), 10000);
            }
        }
        else {
            auto pos = getPos();
            if (zeno::getSession().userData().get2<bool>("viewport-depth-aware-navigation", true)) {
                auto session = m_zenovis->getSession();
                auto scene = session->get_scene();
                auto hit_posWS = scene->renderMan->getEngine()->getClickedPos(x, y);
                if (hit_posWS.has_value()) {
                    auto pivot = hit_posWS.value();
                    auto new_pos = (pos - pivot) * scale + pivot;
                    setPos(new_pos);
                }
                else {
                    auto posOnFloorWS = screenHitOnFloorWS(x / res().x(), y / res().y());
                    auto pivot = posOnFloorWS;
                    if (dot((pivot - pos), getViewDir()) > 0) {
                        auto translate = (pivot - pos) * (1 - scale);
                        if (glm::length(translate) < 0.01) {
                            translate = glm::normalize(translate) * 0.01f;
                        }
                        auto new_pos = translate + pos;
                        setPos(new_pos);
                    }
                    else {
                        auto translate = screenPosToRayWS(x / res().x(), y / res().y()) * getPos().y * (1 - scale);
                        if (getPos().y < 0) {
                            translate *= -1;
                        }
                        auto new_pos = translate + pos;
                        setPos(new_pos);
                    }
                }
            }
            else {
                auto pivot = getPivot();
                auto new_pos = (pos - pivot) * scale + pivot;
                setPos(new_pos);
            }
        }
    }
    updatePerspective();
}

void CameraControl::fakeMouseDoubleClickEvent(ViewMouseInfo info)
{
    auto pos = info.pos;
    auto m_picker = this->m_picker.lock();
    if (!m_picker)
        return;
    auto scene = m_zenovis->getSession()->get_scene();
    auto picked_prim = m_picker->just_pick_prim(pos.x(), pos.y());
    if (!picked_prim.empty()) {
        auto primList = scene->objectsMan->pairs();
        QString mtlid;
        for (auto const &[key, ptr]: primList) {
            if (picked_prim == key) {
                auto ud = ptr->userData();
                mtlid = QString::fromStdString(zsString2Std(ud->get_string("mtlid", "")));
                std::cout<<"selected MatId: "<< zsString2Std(ud->get_string("mtlid", "Default"))<<"\n";
            }
        }

        //TODO: the base tree model is GraphsTreeModel, not plain models any more
        //NEED TO UPDATE THIS.

        /*
        QString subgraph_name;
        QString obj_node_name;
        int type = ZenoSettingsManager::GetInstance().getValue(zsSubgraphType).toInt();
        if (type == SUBGRAPH_TYPE::SUBGRAPH_METERIAL && !mtlid.isEmpty())
        {
            auto graphsMgm = zenoApp->graphsManager();
            IGraphsModel* pModel = graphsMgm->currentModel();

            const auto& lst = pModel->subgraphsIndice(SUBGRAPH_METERIAL);
            for (const auto& index : lst)
            {
                if (index.data(QtRole::ROLE_MTLID).toString() == mtlid)
                    subgraph_name = index.data(QtRole::ROLE_CLASS_NAME).toString();
            }
        }
        if (subgraph_name.isEmpty())
        {
            auto obj_node_location = zeno::NodeSyncMgr::GetInstance().searchNodeOfPrim(picked_prim);
            subgraph_name = obj_node_location->subgraph.data(QtRole::ROLE_CLASS_NAME).toString();
            obj_node_name = obj_node_location->node.data(QtRole::ROLE_NODE_NAME).toString();
        }

        ZenoMainWindow *pWin = zenoApp->getMainWindow();
        if (pWin) {
            ZenoGraphsEditor *pEditor = pWin->getAnyEditor();
            if (pEditor)
                pEditor->activateTab(subgraph_name, "", obj_node_name);
        }
        */
    }
}

void CameraControl::focus(QVector3D center, float radius) {
    setPivot({float(center.x()), float(center.y()), float(center.z())});
    if (getFOV() >= 1e-6)
        radius /= (getFOV() / 45.0f);
    auto dir = getRotation() * glm::vec3(0, 0, 1) * radius;
    setPos(getPivot() + dir);
    updatePerspective();
}

QVector3D CameraControl::realPos() const {
    auto p = getPos();
    return {p[0], p[1], p[2]};
}

// 计算射线与平面的交点
std::optional<glm::vec3> CameraControl::intersectRayPlane(
        glm::vec3 ray_origin
        , glm::vec3 ray_direction
        , glm::vec3 plane_point
        , glm::vec3 plane_normal
) {
    // 计算射线方向和平面法向量的点积
    float denominator = glm::dot(plane_normal, ray_direction);

    // 如果点积接近于0，说明射线与平面平行或在平面内
    if (glm::abs(denominator) < 1e-6f) {
        return std::nullopt; // 返回空，表示没有交点
    }

    // 计算射线起点到平面上一点的向量
    glm::vec3 diff = plane_point - ray_origin;

    // 计算t值
    float t = glm::dot(diff, plane_normal) / denominator;

    // 如果t < 0，说明交点在射线起点之前，返回空

    if (t < 0) {
        return std::nullopt;
    }

    // 计算交点
    glm::vec3 intersection = ray_origin + t * ray_direction;

    return intersection;
}

// x, y from [0, 1]
glm::vec3 CameraControl::screenPosToRayWS(float x, float y)  {
    x = (x - 0.5) * 2;
    y = (y - 0.5) * (-2);
    float v = std::tan(glm::radians(getFOV()) * 0.5f);
    float aspect = res().x() / res().y();
    auto dir = glm::normalize(glm::vec3(v * x * aspect, v * y, -1));
    return getRotation() * dir;
}

glm::vec3 CameraControl::screenHitOnFloorWS(float x, float y) {
    auto dir = screenPosToRayWS(x, y);
    auto pos = getPos();
    float t = (0 - pos.y) / dir.y;
    return pos + dir * t;
}

void CameraControl::fakeMouseReleaseEvent(ViewMouseInfo info) {
    if (info.buttons == Qt::MiddleButton) {
        middle_button_pressed = false;
    }
    if (info.buttons == Qt::LeftButton) {
        auto m_transformer = this->m_transformer.lock();
        auto m_picker = this->m_picker.lock();
        //if (Zenovis::GetInstance().m_bAddPoint == true) {
        //float x = (float)event->x() / m_res.x();
        //float y = (float)event->y() / m_res.y();
        //auto rdir = screenToWorldRay(x, y);
        //auto pos = realPos();
        //float t = (0 - pos.y()) / rdir.y();
        //auto p = pos + rdir * t;

        //float cos_t = std::cos(m_theta);
        //float sin_t = std::sin(m_theta);
        //float cos_p = std::cos(m_phi);
        //float sin_p = std::sin(m_phi);
        //QVector3D back(cos_t * sin_p, sin_t, -cos_t * cos_p);
        //QVector3D up(-sin_t * sin_p, cos_t, sin_t * cos_p);
        //QVector3D right = QVector3D::crossProduct(up, back).normalized();
        //up = QVector3D::crossProduct(right, back).normalized();
        //QVector3D delta = right * x + up * y;

        //zeno::log_info("create point at x={} y={}", p[0], p[1]);

        ////createPointNode(QPointF(p[0], p[1]));

        //Zenovis::GetInstance().m_bAddPoint = false;
        //}

        auto scene = m_zenovis->getSession()->get_scene();
        if (!m_picker || !m_transformer)
            return;

        if (m_transformer->isTransforming()) {
            bool moved = false;
            if (m_boundRectStartPos != info.pos) {
                // create/modify transform primitive node
                moved = true;
            }
            m_transformer->endTransform(moved);
        } else {
            auto cam_pos = realPos();

            scene->select_box = std::nullopt;
            bool ctrl_pressed = info.modifiers & Qt::ControlModifier;
            bool shift_pressed = info.modifiers & Qt::ShiftModifier;
            if (!shift_pressed) {
                scene->selected.clear();
                m_transformer->clear();
            }

            auto onPrimSelected = [this]() {
                auto scene = m_zenovis->getSession()->get_scene();
                ZenoMainWindow *mainWin = zenoApp->getMainWindow();
                mainWin->onPrimitiveSelected(scene->selected);
//                auto pos = event->pos();
//                if (!m_picker)
//                    return;
//
//                auto picked_prim = m_picker->just_pick_prim(pos.x(), pos.y());
//                if (!picked_prim.empty()) {
//                    auto obj_node_location = zeno::NodeSyncMgr::GetInstance().searchNodeOfPrim(picked_prim);
//                    auto subgraph_name = obj_node_location->subgraph.data(QtRole::ROLE_CLASS_NAME).toString();
//                    auto obj_node_name = obj_node_location->node.data(QtRole::ROLE_NODE_NAME).toString();
//                    ZenoMainWindow *pWin = zenoApp->getMainWindow();
//                    if (pWin) {
//                        ZenoGraphsEditor *pEditor = pWin->getAnyEditor();
//                        if (pEditor)
//                            pEditor->activateTab(subgraph_name, "", obj_node_name);
//                    }
//                }
            };

            QPoint releasePos = info.pos.toPoint();
            if (m_boundRectStartPos == releasePos) {
                if (m_picker->is_draw_mode()) {
                    // zeno::log_info("res_w: {}, res_h: {}", res()[0], res()[1]);
                    m_picker->pick_depth(releasePos.x(), releasePos.y());
                } else {
                    m_picker->pick(releasePos.x(), releasePos.y());
                    m_picker->sync_to_scene();
                    if (scene->get_select_mode() == zenovis::PICK_MODE::PICK_OBJECT)
                        onPrimSelected();
                    m_transformer->clear();
                    m_transformer->addObject(m_picker->get_picked_prims());
                }
                for(auto prim:m_picker->get_picked_prims())
                {
                    if (!prim.empty()) {
                        auto primList = scene->objectsMan->pairs();
                        for (auto const &[key, ptr]: primList) {
                            if (prim == key) {
                                auto ud = ptr->userData();
                                auto mtlidstr = ud->get_string("mtlid", "Default");
                                std::cout<<"selected MatId: "<<zsString2Std(mtlidstr)<<"\n";
                            }
                        }
                    }
                }
            } else {
                int x0 = m_boundRectStartPos.x();
                int y0 = m_boundRectStartPos.y();
                int x1 = releasePos.x();
                int y1 = releasePos.y();
                zeno::SELECTION_MODE mode = zeno::SELECTION_MODE::NORMAL;
                if (shift_pressed == false && ctrl_pressed == false) {
                    mode = zeno::SELECTION_MODE::NORMAL;
                }
                else if (shift_pressed == true && ctrl_pressed == false) {
                    mode = zeno::SELECTION_MODE::APPEND;
                }
                else if (shift_pressed == false && ctrl_pressed == true) {
                    mode = zeno::SELECTION_MODE::REMOVE;
                }

                m_picker->pick(x0, y0, x1, y1, mode);
                m_picker->sync_to_scene();
                if (scene->get_select_mode() == zenovis::PICK_MODE::PICK_OBJECT)
                    onPrimSelected();
                m_transformer->clear();
                m_transformer->addObject(m_picker->get_picked_prims());
                std::cout<<"selected items:"<<m_picker->get_picked_prims().size()<<"\n";
                std::vector<QString> nodes;
                QString sgname;
                for(auto prim:m_picker->get_picked_prims())
                {
                    if (!prim.empty()) {
                        auto primList = scene->objectsMan->pairs();
                        for (auto const &[key, ptr]: primList) {
                            if (prim == key) {
                                auto ud = ptr->userData();
                                auto mtlidstr = ud->get_string("mtlid", "Default");
                                std::cout<<"selected MatId: "<< zsString2Std(mtlidstr) <<"\n";
                            }
                        }
                        auto obj_node_location = zeno::NodeSyncMgr::GetInstance().searchNodeOfPrim(prim);
                        if (!obj_node_location)
                        {
                            return;
                        }
                        auto subgraph_name = obj_node_location.value().subgraph->name();
                        auto obj_node_name = obj_node_location->node.data(QtRole::ROLE_NODE_NAME).toString();
                        nodes.push_back(obj_node_name);
//                        ZenoMainWindow *pWin = zenoApp->getMainWindow();
//                        if (pWin) {
//                            ZenoGraphsEditor *pEditor = pWin->getAnyEditor();
//                            if (pEditor)
//                                pEditor->selectTab(subgraph_name, "", obj_node_name);
//                        }
                        sgname = subgraph_name;
                    }
                }

                ZenoMainWindow *pWin = zenoApp->getMainWindow();
                if (pWin) {
                    ZenoGraphsEditor *pEditor = pWin->getAnyEditor();
                    if (pEditor)
                        pEditor->selectTab(sgname, "", nodes);
                }


            }
        }
    }
}

bool CameraControl::fakeKeyPressEvent(int uKey) {
    // viewport focus prim
    if ((uKey & 0xff) == Qt::Key_F && uKey & Qt::AltModifier) {
        auto *scene = m_zenovis->getSession()->get_scene();
        if (scene->selected.size() == 1) {
            std::string nodeId = *scene->selected.begin();
            nodeId = nodeId.substr(0, nodeId.find_first_of(':'));
            zeno::vec3f center;
            float radius;
            if (m_zenovis->getSession()->focus_on_node(nodeId, center, radius)) {
                focus(QVector3D(center[0], center[1], center[2]), radius * 3.0f);
            }
        }
        updatePerspective();
        return true;
    }
    if ((uKey & 0xff) == Qt::Key_G && uKey & Qt::AltModifier) {
        auto *scene = m_zenovis->getSession()->get_scene();
        scene->camera->reset();
        updatePerspective();
        return true;
    }
    if (!middle_button_pressed) {
        return false;
    }
    float step = glm::pow(1.2f, float(zeno::getSession().userData().get2<int>("viewport-FPN-move-speed", 0)));

    bool processed = false;
    if (uKey == Qt::Key_Q) {
        setPos(getPos() - getUpDir() * step);
        processed = true;
    }
    else if (uKey == Qt::Key_E) {
        setPos(getPos() + getUpDir() * step);
        processed = true;
    }
    else if (uKey == Qt::Key_W) {
        setPos(getPos() + getViewDir() * step);
        processed = true;
    }
    else if (uKey == Qt::Key_S) {
        setPos(getPos() - getViewDir() * step);
        processed = true;
    }
    else if (uKey == Qt::Key_A) {
        setPos(getPos() - getRightDir() * step);
        processed = true;
    }
    else if (uKey == Qt::Key_D) {
        setPos(getPos() + getRightDir() * step);
        processed = true;
    }
    if (processed) {
        updatePerspective();
    }
    return processed;
}

bool CameraControl::fakeKeyReleaseEvent(int uKey) {
    return false;
}
//void CameraControl::createPointNode(QPointF pnt) {
//auto pModel = zenoApp->graphsManager()->currentModel();
//ZASSERT_EXIT(pModel);
////todo luzh: select specific subgraph to add node.
//const QModelIndex &subgIdx = pModel->index("main");
//NODE_DATA tmpNodeInfo = NodesMgr::createPointNode(pModel, subgIdx, "CreatePoint", {10, 10}, pnt);

//STATUS_UPDATE_INFO info;
//info.role = ROLE_OPTIONS;
//info.newValue = OPT_VIEW;
//pModel->updateNodeStatus(tmpNodeInfo[QtRole::ROLE_NODE_NAME].toString(), info, subgIdx, true);
//}
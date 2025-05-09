#include <zenovis/RenderEngine.h>
#include "viewportwidget.h"
#include "zenovis.h"
#include "camerakeyframe.h"
#include "widgets/ztimeline.h"
#include "zenomainwindow.h"
#include "dialog/zrecorddlg.h"
#include "dialog/zrecprogressdlg.h"
#include <zeno/utils/log.h>
#include <zenovis/ObjectsManager.h>
#include <zenovis/DrawOptions.h>
#include <zeno/funcs/ObjectGeometryInfo.h>
#include <util/log.h>
#include "style/zenostyle.h"
#include "nodeeditor/gv/zenographseditor.h"
#include <cmath>
#include <algorithm>
#include <optional>
#include <zeno/core/Session.h>
#include <zeno/extra/GlobalState.h>
#include <zeno/extra/GlobalComm.h>
#include "util/uihelper.h"
#include <zeno/types/UserData.h>
#include "settings/zenosettingsmanager.h"
#include "cameracontrol.h"
#include "style/dpiscale.h"


using std::string;
using std::unordered_set;
using std::unordered_map;


ViewportWidget::ViewportWidget(QWidget* parent)
    : QGLWidget(parent)
    , m_camera(nullptr)
    , m_pauseRenderDally(new QTimer)
    , m_wheelEventDally(new QTimer)
    , simpleRenderTime(0)
    , m_bMovingCamera(false)
    , m_zenovis(nullptr)
{
    m_zenovis = new Zenovis(this);
    m_picker = std::make_shared<zeno::Picker>(this);
    m_fakeTrans = std::make_shared<zeno::FakeTransformer>(this);

    QGLFormat fmt;
    int nsamples = 16;  // TODO: adjust in a zhouhang-panel
    fmt.setSamples(nsamples);
    fmt.setVersion(3, 2);
    fmt.setProfile(QGLFormat::CoreProfile);
    setFormat(fmt);

    // https://blog.csdn.net/zhujiangm/article/details/90760744
    // https://blog.csdn.net/jays_/article/details/83783871
    setFocusPolicy(Qt::ClickFocus);
    setMouseTracking(true);

    m_camera = new CameraControl(m_zenovis, m_fakeTrans, m_picker, this);
    m_zenovis->m_camera_control = m_camera;

    connect(m_zenovis, &Zenovis::objectsUpdated, this, [=](int frameid) {
        auto mainWin = zenoApp->getMainWindow();
        if (mainWin)
            emit mainWin->visObjectsUpdated(this, frameid);
    });

    connect(m_zenovis, &Zenovis::frameUpdated, this, [=](int frameid) {
        auto mainWin = zenoApp->getMainWindow();
        if (mainWin) {
            mainWin->visFrameUpdated(true, frameid);
        }
        clearTransformer();
    });

    connect(m_pauseRenderDally, &QTimer::timeout, [&](){
        auto scene = m_zenovis->getSession()->get_scene();
        scene->drawOptions->simpleRender = false;
        scene->drawOptions->needRefresh = true;
        m_pauseRenderDally->stop();
        //std::cout << "SR: SimpleRender false, Active " << m_pauseRenderDally->isActive() << "\n";
    });

    connect(m_wheelEventDally, &QTimer::timeout, [&](){
        m_wheelEventDally->stop();
        m_bMovingCamera = false;
    });
}

void ViewportWidget::setSimpleRenderOption() {
    //if(simpleRenderChecked)
    //    return;

    auto scene = m_zenovis->getSession()->get_scene();
    scene->drawOptions->simpleRender = true;
    m_pauseRenderDally->stop();
    m_pauseRenderDally->start(simpleRenderTime*1000);  // Second to millisecond
}

void ViewportWidget::setViewWidgetInfo(DockContentWidgetInfo& info)
{
    std::get<0>(viewInfo) = info.resolutionX;
    std::get<1>(viewInfo) = info.resolutionY;
    std::get<2>(viewInfo) = info.lock;
    std::get<3>(viewInfo) = info.colorR;
    std::get<4>(viewInfo) = info.colorG;
    std::get<5>(viewInfo) = info.colorB;
    loadSettingFromZsg = true;
}

void ViewportWidget::glDrawForCommandLine()
{
    QGLWidget::glDraw();
}

ViewportWidget::~ViewportWidget()
{
    //testCleanUp();
    delete m_pauseRenderDally;
    delete m_wheelEventDally;
}

void ViewportWidget::testCleanUp()
{
    delete m_camera;
    delete m_zenovis;
    m_camera = nullptr;
    m_zenovis = nullptr;
    m_picker.reset();
    m_fakeTrans.reset();
}

void ViewportWidget::cleanUpView()
{
    if (m_zenovis)
        m_zenovis->cleanupView();

    m_picker = nullptr;
    m_fakeTrans = nullptr;
}

namespace {
struct OpenGLProcAddressHelper {
    inline static QGLContext *ctx;

    static void *getProcAddress(const char *name) {
        return (void *)ctx->getProcAddress(name);
    }
};
}

void ViewportWidget::initializeGL()
{
    OpenGLProcAddressHelper::ctx = context();
    ZASSERT_EXIT(m_zenovis);
    m_zenovis->loadGLAPI((void *)OpenGLProcAddressHelper::getProcAddress);
    m_zenovis->initializeGL();
    ZASSERT_EXIT(m_picker);
    m_picker->initialize();
    if (loadSettingFromZsg)
    {
        auto session = m_zenovis->getSession();
        ZASSERT_EXIT(session);
        auto scene = session->get_scene();
        ZASSERT_EXIT(scene);
        scene->camera->setResolutionInfo(std::get<2>(viewInfo), std::get<0>(viewInfo), std::get<1>(viewInfo));
        session->set_background_color(std::get<3>(viewInfo), std::get<4>(viewInfo), std::get<5>(viewInfo));
    }
}

void ViewportWidget::resizeGL(int nx, int ny)
{
#ifdef ENABLE_HIGHDPI_SCALE
    float ratio = 1.0;
#else
    float ratio = devicePixelRatioF();
#endif
    zeno::log_trace("nx={}, ny={}, dpr={}", nx, ny, ratio);
    m_camera->setRes(QVector2D(nx * ratio, ny * ratio));
    m_camera->updatePerspective();
}

QVector2D ViewportWidget::cameraRes() const
{
    return m_camera->res();
}

Zenovis* ViewportWidget::getZenoVis() const
{
    return m_zenovis;
}

std::shared_ptr<zeno::Picker> ViewportWidget::picker() const
{
    return m_picker;
}

std::shared_ptr<zeno::FakeTransformer> ViewportWidget::fakeTransformer() const
{
    return m_fakeTrans;
}

zenovis::Session* ViewportWidget::getSession() const
{
    return m_zenovis->getSession();
}

bool ViewportWidget::isPlaying() const
{
    return m_zenovis->isPlaying();
}

bool ViewportWidget::isCameraMoving() const {
    return m_bMovingCamera;
}

void ViewportWidget::startPlay(bool bPlaying)
{
    m_zenovis->startPlay(bPlaying);
}

void ViewportWidget::setCameraRes(const QVector2D& res)
{
    m_camera->setRes(res);
}

void ViewportWidget::setSafeFrames(bool bLock, int nx, int ny)
{
    auto scene = m_zenovis->getSession()->get_scene();
    scene->camera->set_safe_frames(bLock, nx, ny);
    update();
}

void ViewportWidget::updatePerspective()
{
    m_camera->updatePerspective();
}

void ViewportWidget::load_objects()
{
#if 0
    zeno::RenderObjsInfo objs;
    zeno::getSession().objsMan->export_loading_objs(objs);
    m_zenovis->load_objects(objs);
#endif
    std::vector<zeno::render_update_info> infos;
    zeno::getSession().objsMan->export_render_infos(infos);
    m_zenovis->load_objects(infos);
}

void ViewportWidget::load_object(zeno::render_update_info info)
{
    m_zenovis->load_object(info);
}

void ViewportWidget::reload_objects(const zeno::render_reload_info& info)
{
    m_zenovis->reload(info);
}

void ViewportWidget::paintGL()
{
    m_zenovis->paintGL();
}

void ViewportWidget::mousePressEvent(QMouseEvent* event)
{
    int button = Qt::NoButton;
    ZenoSettingsManager& settings = ZenoSettingsManager::GetInstance();
    settings.getViewShortCut(ShortCut_MovingView, button);
    settings.getViewShortCut(ShortCut_RotatingView, button);
    if(event->button() & button){
        m_bMovingCamera = true;
        setSimpleRenderOption();
    }
    _base::mousePressEvent(event);

    ViewMouseInfo info = { event->type(), event->modifiers(), event->buttons(), event->pos() };
    m_camera->fakeMousePressEvent(info);
    update();
}

void ViewportWidget::mouseMoveEvent(QMouseEvent* event)
{
    int button = Qt::NoButton;
    ZenoSettingsManager& settings = ZenoSettingsManager::GetInstance();
    settings.getViewShortCut(ShortCut_MovingView, button);
    settings.getViewShortCut(ShortCut_RotatingView, button);
    if(event->button() & button){
        m_bMovingCamera = true;
    }
    setSimpleRenderOption();
    _base::mouseMoveEvent(event);

    ViewMouseInfo info = { event->type(), event->modifiers(), event->buttons(), event->pos() };
    m_camera->fakeMouseMoveEvent(info);
    update();
}

void ViewportWidget::wheelEvent(QWheelEvent* event)
{
    m_bMovingCamera = true;
    m_wheelEventDally->start(100);
    setSimpleRenderOption();
    _base::wheelEvent(event);

    ViewMouseInfo info = { event->type(), event->modifiers(), event->buttons(), event->pos(), event->angleDelta() };
    m_camera->fakeWheelEvent(info);
    update();
}

void ViewportWidget::mouseReleaseEvent(QMouseEvent *event) {
    if(event->button() == Qt::MidButton){
        m_bMovingCamera = false;
    }

    _base::mouseReleaseEvent(event);
    ViewMouseInfo info = { event->type(), event->modifiers(), event->buttons(), event->pos() };
    m_camera->fakeMouseReleaseEvent(info); 
    update();
}

void ViewportWidget::mouseDoubleClickEvent(QMouseEvent* event) {
    _base::mouseReleaseEvent(event);
    ViewMouseInfo info = { event->type(), event->modifiers(), event->buttons(), event->pos() };
    m_camera->fakeMouseDoubleClickEvent(info);
    update();
}

//void ViewportWidget::mouseDoubleClickEvent(QMouseEvent* event) {
void ViewportWidget::cameraLookTo(zenovis::CameraLookToDir dir) {
     m_camera->lookTo(dir);
}


void ViewportWidget::clearTransformer() {
    m_camera->clearTransformer();
}

void ViewportWidget::changeTransformOperation(const QString& node) {
    m_camera->changeTransformOperation(node);
}

void ViewportWidget::changeTransformOperation(int mode) {
    m_camera->changeTransformOperation(mode);
}

void ViewportWidget::changeTransformCoordSys() {
    m_camera->changeTransformCoordSys();
}

void ViewportWidget::cleanUpScene() {
    if (!m_zenovis)
        return;
    m_zenovis->cleanUpScene();
}

void ViewportWidget::updateCameraProp(float aperture, float disPlane) {
    m_camera->setAperture(aperture);
    m_camera->setDisPlane(disPlane);
    updatePerspective();
}

void ViewportWidget::setNumSamples(int samples)
{
    auto scene = getSession()->get_scene();
    if (scene) {
        scene->drawOptions->num_samples = samples;
    }
}

void ViewportWidget::keyPressEvent(QKeyEvent *event)
{
    _base::keyPressEvent(event);
    //qInfo() << event->key();
    ZenoSettingsManager &settings = ZenoSettingsManager::GetInstance();
    int key = settings.getShortCut(ShortCut_MovingHandler);
    int uKey = event->key();
    Qt::KeyboardModifiers modifiers = event->modifiers();
    if (modifiers & Qt::ShiftModifier) {
        uKey += Qt::SHIFT;
    }
    if (modifiers & Qt::ControlModifier) {
        uKey += Qt::CTRL;
    }
    if (modifiers & Qt::AltModifier) {
        uKey += Qt::ALT;
    }

    if (m_camera->fakeKeyPressEvent(uKey)) {
        zenoApp->getMainWindow()->updateViewport();
        return;
    }

    if (uKey == key)
        this->changeTransformOperation(0);
    key = settings.getShortCut(ShortCut_RevolvingHandler);
    if (uKey == key)
        this->changeTransformOperation(1);
    key = settings.getShortCut(ShortCut_ScalingHandler);
    if (uKey == key)
        this->changeTransformOperation(2);
    key = settings.getShortCut(ShortCut_CoordSys);
    if (uKey == key)
        this->changeTransformCoordSys();

    key = settings.getShortCut(ShortCut_FrontView);
    if (uKey == key)
        this->cameraLookTo(zenovis::CameraLookToDir::front_view);
    key = settings.getShortCut(ShortCut_RightView);
    if (uKey == key)
        this->cameraLookTo(zenovis::CameraLookToDir::right_view);
    key = settings.getShortCut(ShortCut_VerticalView);
    if (uKey == key)
        this->cameraLookTo(zenovis::CameraLookToDir::top_view);
    key = settings.getShortCut(ShortCut_InitViewPos);
    if (uKey == key)
        this->cameraLookTo(zenovis::CameraLookToDir::back_to_origin);

    key = settings.getShortCut(ShortCut_BackView);
    if (uKey == key)
        this->cameraLookTo(zenovis::CameraLookToDir::back_view);
    key = settings.getShortCut(ShortCut_LeftView);
    if (uKey == key)
        this->cameraLookTo(zenovis::CameraLookToDir::left_view);
    key = settings.getShortCut(ShortCut_UpwardView);
    if (uKey == key)
        this->cameraLookTo(zenovis::CameraLookToDir::bottom_view);

    key = settings.getShortCut(ShortCut_InitHandler);
    if (uKey == key)
        m_camera->resizeTransformHandler(0);
    key = settings.getShortCut(ShortCut_AmplifyHandler);
    if (uKey == key)
        m_camera->resizeTransformHandler(1);
    key = settings.getShortCut(ShortCut_ReduceHandler);
    if (uKey == key)
        m_camera->resizeTransformHandler(2);
}

void ViewportWidget::keyReleaseEvent(QKeyEvent *event) {
    _base::keyReleaseEvent(event);
    int uKey = event->key();
    if (m_camera->fakeKeyReleaseEvent(uKey)) {
        zenoApp->getMainWindow()->updateViewport();
        return;
    }
}

void ViewportWidget::enterEvent(QEvent *event) {
    //setFocus();
    QWidget::enterEvent(event);
}

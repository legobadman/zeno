#include "zopenglquickview.h"
#include "camera.h"
#include "zqmlrender.h"
#include <QSurfaceFormat>
#include <QQmlContext>
#include "settings/zenosettingsmanager.h"


ZOpenGLQuickView::ZOpenGLQuickView(QWindow* parent)
    : QQuickView(parent)
#ifdef BASE_KDAB
    , m_camera(new Camera(this))
#endif
    , m_renderer(new ZQmlRender(this))
{
    QSurfaceFormat format;
    format.setMajorVersion(3);
    format.setMinorVersion(3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setSamples(4);
    setFormat(format);

    connect(this, &QQuickWindow::sceneGraphInitialized,
        this, &ZOpenGLQuickView::initializeUnderlay,
        Qt::DirectConnection);

    connect(this, &QQuickWindow::beforeSynchronizing,
        this, &ZOpenGLQuickView::synchronizeUnderlay,
        Qt::DirectConnection);

    connect(this, &QQuickWindow::beforeRendering,
        this, &ZOpenGLQuickView::renderUnderlay,
        Qt::DirectConnection);

    connect(this, &QQuickWindow::sceneGraphInvalidated,
        this, &ZOpenGLQuickView::invalidateUnderlay,
        Qt::DirectConnection);

#ifdef BASE_KDAB
    connect(m_camera, &Camera::azimuthChanged,
        this, &QQuickWindow::update);

    connect(m_camera, &Camera::elevationChanged,
        this, &QQuickWindow::update);

    connect(m_camera, &Camera::distanceChanged,
        this, &QQuickWindow::update);

    rootContext()->setContextProperty("_camera", m_camera);
    setSource(QUrl("qrc:///qml/mainview.qml"));
#endif
    setClearBeforeRendering(false);
    setPersistentOpenGLContext(true);
    setResizeMode(SizeRootObjectToView);
}

void ZOpenGLQuickView::initializeUnderlay() {
    auto ctx = this->openglContext();
    m_renderer->initialize(ctx);
    resetOpenGLState();
}

void ZOpenGLQuickView::synchronizeUnderlay() {
#ifdef BASE_KDAB
    m_renderer->setAzimuth(m_camera->azimuth());
    m_renderer->setElevation(m_camera->elevation());
    m_renderer->setDistance(m_camera->distance());
#else
    if (m_cache_info.policy != zeno::Reload_Invalidate) {
        m_renderer->reload_objects(m_cache_info);
        m_cache_info.policy = zeno::Reload_Invalidate;
    }
#endif
}

void ZOpenGLQuickView::reload_objects(const zeno::render_reload_info& info) {
    m_cache_info = info;
}

void ZOpenGLQuickView::renderUnderlay() {
    m_renderer->render();
    resetOpenGLState();
}

void ZOpenGLQuickView::invalidateUnderlay() {
    m_renderer->invalidate();
    resetOpenGLState();
}

void ZOpenGLQuickView::mousePressEvent(QMouseEvent* event)
{
    int button = Qt::NoButton;
    ZenoSettingsManager& settings = ZenoSettingsManager::GetInstance();
    settings.getViewShortCut(ShortCut_MovingView, button);
    settings.getViewShortCut(ShortCut_RotatingView, button);
    if (event->button() & button) {
        //m_bMovingCamera = true;
        //setSimpleRenderOption();
    }
    QQuickView::mousePressEvent(event);
    m_renderer->fakeMousePressEvent(event);
    update();
}

void ZOpenGLQuickView::mouseReleaseEvent(QMouseEvent* event)
{
    QQuickView::mouseReleaseEvent(event);
    m_renderer->fakeMouseReleaseEvent(event);
    update();
}

void ZOpenGLQuickView::mouseMoveEvent(QMouseEvent* event)
{
    //int button = Qt::NoButton;
    //ZenoSettingsManager& settings = ZenoSettingsManager::GetInstance();
    //settings.getViewShortCut(ShortCut_MovingView, button);
    //settings.getViewShortCut(ShortCut_RotatingView, button);
    //if (event->button() & button) {
    //    m_bMovingCamera = true;
    //}
    //setSimpleRenderOption();
    QQuickView::mouseMoveEvent(event);
    m_renderer->fakeMouseMoveEvent(event);
    update();
}

void ZOpenGLQuickView::wheelEvent(QWheelEvent* event)
{
    QQuickView::wheelEvent(event);
    m_renderer->fakeWheelEvent(event);
    update();
}

void ZOpenGLQuickView::resizeEvent(QResizeEvent* event) {
    //moc QGLWidget::resizeEvent
    QQuickView::resizeEvent(event);
    QSize sz = event->size();
    const qreal scaleFactor = devicePixelRatio();
    m_renderer->resize(sz.width() * scaleFactor, sz.height() * scaleFactor);
}

void ZOpenGLQuickView::resizeGL(int nx, int ny) {
    m_renderer->resize(nx, ny);
}

void ZOpenGLQuickView::updatePerspective()
{
    m_renderer->updatePerspective();
}

void ZOpenGLQuickView::setNumSamples(int samples) {
    m_renderer->setNumSamples(samples);
}

void ZOpenGLQuickView::setCameraRes(const QVector2D& res) {
    m_renderer->setCameraRes(res);
}

void ZOpenGLQuickView::setSafeFrames(bool bLock, int nx, int ny) {
    m_renderer->setSafeFrames(bLock, nx, ny);
}

bool ZOpenGLQuickView::isCameraMoving() const {
    return false;
}

void ZOpenGLQuickView::setSimpleRenderOption()
{
    m_renderer->setSimpleRenderOption();
}

void ZOpenGLQuickView::clearTransformer()
{
    m_renderer->clearTransformer();
}

void ZOpenGLQuickView::changeTransformOperation(const QString& node)
{
    m_renderer->changeTransformOperation(node);
}

void ZOpenGLQuickView::changeTransformOperation(int mode)
{
    m_renderer->changeTransformOperation(mode);
}

void ZOpenGLQuickView::cameraLookTo(zenovis::CameraLookToDir dir)
{
    m_renderer->cameraLookTo(dir);
}

void ZOpenGLQuickView::setViewWidgetInfo(DockContentWidgetInfo& info)
{
    m_renderer->setViewWidgetInfo(info);
}

ZOpenGLQuickWidget::ZOpenGLQuickWidget(QWidget* parent)
    : QWidget(parent)
{

}
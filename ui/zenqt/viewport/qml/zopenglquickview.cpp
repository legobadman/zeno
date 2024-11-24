#include "zopenglquickview.h"
#include "camera.h"
#include "zqmlrender.h"
#include <QSurfaceFormat>
#include <QQmlContext>


ZOpenGLQuickView::ZOpenGLQuickView(QWindow* parent)
    : QQuickView(parent)
    , m_camera(new Camera(this))
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

    connect(m_camera, &Camera::azimuthChanged,
        this, &QQuickWindow::update);

    connect(m_camera, &Camera::elevationChanged,
        this, &QQuickWindow::update);

    connect(m_camera, &Camera::distanceChanged,
        this, &QQuickWindow::update);

    setClearBeforeRendering(false);
    setPersistentOpenGLContext(true);

    setResizeMode(SizeRootObjectToView);
    rootContext()->setContextProperty("_camera", m_camera);
    setSource(QUrl("qrc:///qml/mainview.qml"));
}

void ZOpenGLQuickView::initializeUnderlay() {
    m_renderer->initialize();
    resetOpenGLState();
}

void ZOpenGLQuickView::synchronizeUnderlay() {
    m_renderer->setAzimuth(m_camera->azimuth());
    m_renderer->setElevation(m_camera->elevation());
    m_renderer->setDistance(m_camera->distance());
}

void ZOpenGLQuickView::renderUnderlay() {
    m_renderer->render();
    resetOpenGLState();
}

void ZOpenGLQuickView::invalidateUnderlay() {
    m_renderer->invalidate();
    resetOpenGLState();
}


ZOpenGLQuickWidget::ZOpenGLQuickWidget(QWidget* parent)
    : QWidget(parent)
{

}
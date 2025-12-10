#include <QApplication>
#include "style/zenostyle.h"
#include "zenoapplication.h"
#include "zenomainwindow.h"
#include "startup/zstartup.h"
#include "settings/zsettings.h"
#include "zeno/utils/log.h"
#include "zeno/extra/EventCallbacks.h"
#include <GL/glut.h>
#include <QuickQanava>
#include <QQuickStyle>
#include "viewport/qml/zopenglquickview.h"
#include "style/dpiscale.h"


int main_editor(int argc, char* argv[])
{
#ifdef ENABLE_HIGHDPI_SCALE
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    glutInit(&argc, argv);

    ZenoApplication a(argc, argv);
    a.setStyle(new ZenoStyle);

    //ZOpenGLQuickView view;
    ////view.show();
    ////view.resize(600, 600);

    //QWidget* wid = QWidget::createWindowContainer(&view);
    //wid->show();
    //wid->resize(600, 600);
    //return a.exec();

    QTranslator t;
    QTranslator qtTran;
    QSettings settings(zsCompanyName, zsEditor);
    QVariant use_chinese = settings.value("use_chinese");

    if (use_chinese.isNull() || use_chinese.toBool()) {
        if (t.load(":languages/zh.qm")) {
            a.installTranslator(&t);
        }
        if (qtTran.load(":languages/qt_zh_CN.qm")) {
            a.installTranslator(&qtTran);
        }
    }

    startUp(true);

    ZenoMainWindow mainWindow;
    zeno::getSession().eventCallbacks->triggerEvent("editorConstructed");
    mainWindow.showMaximized();
    return a.exec();
}
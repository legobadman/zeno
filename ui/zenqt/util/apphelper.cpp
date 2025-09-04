#include "apphelper.h"
#include "uicommon.h"
#include "../startup/zstartup.h"


QString AppHelper::nativeWindowTitle(const QString& currentFilePath)
{
    QString ver = QString::fromStdString(getZenoVersion());
    if (currentFilePath.isEmpty())
    {
        QString title = QString("Zeno Editor (%1)").arg(ver);
        return title;
    }
    else
    {
        QString title = QString::fromUtf8("%1 - Zeno Editor (%2)").arg(currentFilePath).arg(ver);
        return title;
    }
}

VideoRecInfo AppHelper::getRecordInfo(const ZENO_RECORD_RUN_INITPARAM& param)
{
    VideoRecInfo recInfo;
    recInfo.bitrate = param.iBitrate;
    recInfo.fps = param.iFps;
    recInfo.frameRange = { param.iSFrame, param.iSFrame + param.iFrame - 1 };
    recInfo.numMSAA = 0;
    recInfo.numOptix = param.iSample;
    recInfo.audioPath = param.audioPath;
    recInfo.record_path = param.sPath;
    recInfo.videoname = param.videoName;
    recInfo.bExportVideo = param.isExportVideo;
    recInfo.needDenoise = param.needDenoise;
    recInfo.exitWhenRecordFinish = param.exitWhenRecordFinish;

    if (!param.sPixel.isEmpty())
    {
        QStringList tmpsPix = param.sPixel.split("x");
        int pixw = tmpsPix.at(0).toInt();
        int pixh = tmpsPix.at(1).toInt();
        recInfo.res = { (float)pixw, (float)pixh };

        //viewWidget->setFixedSize(pixw, pixh);
        //viewWidget->setCameraRes(QVector2D(pixw, pixh));
        //viewWidget->updatePerspective();
    }
    else {
        recInfo.res = { (float)1000, (float)680 };
        //viewWidget->setMinimumSize(1000, 680);
    }

    return recInfo;
}

void AppHelper::dumpTabsToZsg(QDockWidget* dockWidget, RAPIDJSON_WRITER& writer)
{
    //not QDockWidget but ads::CDockWidget
    //TODO: refacror.
#if 0
    if (ZDockWidget* tabDockwidget = qobject_cast<ZDockWidget*>(dockWidget))
    {
        for (int i = 0; i < tabDockwidget->count(); i++)
        {
            QWidget* wid = tabDockwidget->widget(i);
            if (qobject_cast<DockContent_Parameter*>(wid)) {
                writer.String("Parameter");
            }
            else if (qobject_cast<DockContent_Editor*>(wid)) {
                writer.String("Editor");
            }
            else if (qobject_cast<DockContent_View*>(wid)) {
                DockContent_View* pView = qobject_cast<DockContent_View*>(wid);
                auto dpwid = pView->getDisplayWid();
                ZASSERT_EXIT(dpwid);
                auto vis = dpwid->getZenoVis();
                ZASSERT_EXIT(vis);
                auto session = vis->getSession();
                ZASSERT_EXIT(session);
                writer.StartObject();
                if (pView->isGLView()) {
                    auto [r, g, b] = session->get_background_color();
                    writer.Key("type");
                    writer.String("View");
                    writer.Key("backgroundcolor");
                    writer.StartArray();
                    writer.Double(r);
                    writer.Double(g);
                    writer.Double(b);
                    writer.EndArray();
                }
                else {
                    writer.Key("type");
                    writer.String("Optix");
                }
                std::tuple<int, int, bool> displayinfo = pView->getOriginWindowSizeInfo();
                writer.Key("blockwindow");
                writer.Bool(std::get<2>(displayinfo));
                writer.Key("resolutionX");
                writer.Int(std::get<0>(displayinfo));
                writer.Key("resolutionY");
                writer.Int(std::get<1>(displayinfo));
                writer.Key("resolution-combobox-index");
                writer.Int(pView->curResComboBoxIndex());
                writer.EndObject();
            }
            else if (qobject_cast<DockContent_Log*>(wid)) {
                writer.String("Logger");
            }
        }
    }
#endif
}
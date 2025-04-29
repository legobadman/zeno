#ifndef __RECORD_VIDEO_MGR_H__
#define __RECORD_VIDEO_MGR_H__

#include <QtWidgets>
#include "common.h"

struct VideoRecInfo
{
    QString record_path;    //store screenshot img and mp4.
    QString audioPath;
    QString videoname;
    QVector2D res = { 0,0 };
    QPair<int, int> frameRange = { -1, -1 };
    QMap<int, bool> m_bFrameFinished;
    int fps = 0;
    int bitrate = 0;
    int numMSAA = 0;
    int numOptix = 1;
    bool bExportVideo = false;
    bool needDenoise = false;
    bool exitWhenRecordFinish = false;
    bool bRecordByCommandLine = false;
    bool bAutoRemoveCache = false;
    bool bExportEXR = false;
};
Q_DECLARE_METATYPE(VideoRecInfo);

class Zenovis;

class RecordVideoMgr : public QObject
{
    Q_OBJECT
public:
    RecordVideoMgr(QObject* parent = nullptr);
    ~RecordVideoMgr();
    void setRecordInfo(const VideoRecInfo& recInfo);
    void initRecordInfo(const VideoRecInfo& recInfo);
    VideoRecInfo getRecordInfo() const;

public slots:
    void cancelRecord();
    void onFrameDrawn(int);
    REC_RETURN_CODE endRecToExportVideo();

signals:
    void frameFinished(int);
    void recordFinished(QString);
    void recordFailed(QString);

private:
    Zenovis* getZenovis();
    void disconnectSignal();
    void recordErrorImg(int currFrame);

    VideoRecInfo m_recordInfo;
    QStringList m_pics;
    int m_currFrame;
};


#endif
#ifndef __ZENO_UI_COMMON_H__
#define __ZENO_UI_COMMON_H__

#include <QModelIndex>
#include <QSize>
#include <QDockWidget>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>
#include <memory>
#include <zeno/core/data.h>

typedef rapidjson::PrettyWriter<rapidjson::StringBuffer> RAPIDJSON_WRITER;

struct LayerOutNode;

struct TIMELINE_INFO
{
    int beginFrame;
    int endFrame;
    int currFrame;
    bool bAlways;
    int timelinefps;

    TIMELINE_INFO() : beginFrame(0), endFrame(0), currFrame(0), bAlways(false), timelinefps(24) {}
};

enum REC_RETURN_CODE
{
    REC_NOERROR = 0,
    REC_NO_RECORD_OPTION = 1,
    REC_OPTIX_INTERNAL_FATAL = -1,
    REC_FFMPEG_FATAL = -3,
    REC_NOFFMPEG = -2,
};

struct RECORD_SETTING
{
    QString record_path;
    QString videoname;
    int fps;
    int bitrate;
    int numMSAA;
    int numOptix;
    int width;
    int height;
    bool bExportVideo;
    bool needDenoise;
    bool bAutoRemoveCache;
    bool bAov;
    bool bMask;
    bool bExr;

    QString exePath;

    RECORD_SETTING() : fps(24), bitrate(200000), numMSAA(0), numOptix(1), width(1280), height(720), bExportVideo(false), needDenoise(false), bAutoRemoveCache(true), bAov(false), bExr(false), bMask(false){}
};

struct LAYOUT_SETTING {
    std::shared_ptr<LayerOutNode> layerOutNode;
    QSize size;
    void(*cbDumpTabsToZsg)(QDockWidget*, RAPIDJSON_WRITER&);
};

struct USERDATA_SETTING
{
    bool optix_show_background;
    USERDATA_SETTING() : optix_show_background(false) {}
};

struct APP_SETTINGS
{
    zeno::TimelineInfo timeline;
    RECORD_SETTING recordInfo;
    LAYOUT_SETTING layoutInfo;
    USERDATA_SETTING userdataInfo;
    //todo: other settings.
};

enum SearchType
{
    SEARCH_SUBNET = 1 << 0,
    SEARCH_NODECLS = 1 << 1,
    SEARCH_NODEID = 1 << 2,		// search node ident.
    SEARCH_ANNO = 1 << 3,
    SEARCH_ARGS = 1 << 4,       // all args.
    SEARCH_CUSTOM_NAME = 1 << 5, 
    SEARCHALL = SEARCH_NODECLS | SEARCH_NODEID | SEARCH_SUBNET | SEARCH_ANNO | SEARCH_ARGS | SEARCH_CUSTOM_NAME
};

enum SearchOpt
{
    SEARCH_FUZZ = 1 << 0,
    SEARCH_MATCH_EXACTLY = 1 << 1,
    SEARCH_CASE_SENSITIVE= 1 << 2,
};

struct SEARCH_RESULT
{
    SearchType type;
    QModelIndex targetIdx;  //node or subgraph index.
    QModelIndex subgIdx;
    QString socket;     //the socket/param which contains the result.
};

struct LiveObjectData{
    std::string verSrc = "";
    std::string camSrc = "";
    int verLoadCount = 0;
    int camLoadCount = 0;
};

struct ZENO_RECORD_RUN_INITPARAM {
    QString sZsgPath = "";
    bool bRecord = false;
    bool bOptix = false;    //is optix view.
    bool isExportVideo = false;
    bool needDenoise = false;
    bool export_exr = false;
    bool bAov = false;
    int iFrame = 0;
    int iSFrame = 0;
    int iSample = 0;
    int iBitrate = 20000;
    int iFps = 24;
    QString sPixel = "";
    QString sPath = "";
    QString audioPath = "";
    QString configFilePath = "";
    QString videoName = "output.mp4";
    QString subZsg = "";
    QString paramsJson = "";
    bool exitWhenRecordFinish = false;
};

#endif

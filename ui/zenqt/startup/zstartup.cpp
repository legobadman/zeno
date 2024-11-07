#include <zeno/extra/EventCallbacks.h>
#include <zeno/extra/assetDir.h>
#include <zeno/core/Session.h>
#include <zeno/utils/log.h>
#include <zeno/types/UserData.h>
#include <zeno/types/GenericObject.h>
#include "zstartup.h"
#include <QApplication>
#include <QSettings>
#include <algorithm>
#include "settings/zsettings.h"
#include "exceptionhandle.h"
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "uicommon.h"
#include "model/LinkModel.h"


void initQml()
{
    qRegisterMetaType<LinkModel*>("LinkModel*");
    qmlRegisterUncreatableType<QmlParamControl>("zeno.enum", 1, 0, "ParamControl", "Not creatable as it is an enum type");
}

void startUp(bool bEnableCrashReport)
{
#ifdef Q_OS_WIN
    if (bEnableCrashReport)
        registerExceptionFilter();
    SetConsoleOutputCP(CP_UTF8);
#endif

    zeno::setExecutableDir(QCoreApplication::applicationDirPath().toStdString());
    zeno::setConfigVariable("EXECFILE", QCoreApplication::applicationFilePath().toStdString());

    initQml();

    QSettings settings(zsCompanyName, zsEditor);

    static int calledOnce = ([]{
        zeno::getSession().eventCallbacks->triggerEvent("init");
    }(), 0);

    QDir docDir(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
    docDir.mkpath("Zeno/assets");
}

std::string getZenoVersion() {
    const char *date = __DATE__;
    const char *table[] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
    };
    int month = std::find(table, table + 12, std::string(date, 3)) - table + 1;
    int day = std::stoi(std::string(date + 4, 2));
    int year = std::stoi(std::string(date + 7, 4));

#ifdef ZENO_WIN32_RC
    TCHAR szFilename[MAX_PATH + 1] = {0};
    if (GetModuleFileName(NULL, szFilename, MAX_PATH) == 0)
    {
        return "";
    }

    // allocate a block of memory for the version info
    DWORD dummy;
    DWORD dwSize = GetFileVersionInfoSize(szFilename, &dummy);
    if (dwSize == 0) {
        zeno::log_error("GetFileVersionInfoSize failed with error %d\n", GetLastError());
        return "";
    }
    std::vector<BYTE> data(dwSize);
    // load the version info
    if (!GetFileVersionInfo(szFilename, NULL, dwSize, &data[0])) {
        zeno::log_error("GetFileVersionInfo failed with error %d\n", GetLastError());
        return "";
    }

    // get the name and version strings
    LPVOID pvProductName = NULL;
    unsigned int iProductNameLen = 0;
    LPVOID pvProductVersion = NULL;
    unsigned int iProductVersionLen = 0;

    // replace "080404b0" with the language ID of your resources
    if (!VerQueryValue(&data[0], "\\StringFileInfo\\080404b0\\ProductName", &pvProductName, &iProductNameLen) ||
        !VerQueryValue(&data[0], "\\StringFileInfo\\080404b0\\ProductVersion", &pvProductVersion,
                       &iProductVersionLen)) {
        zeno::log_error("Can't obtain ProductName and ProductVersion from resources\n");
        return "";
    }
    if (!pvProductVersion)
        return "";

    std::string productVersion((LPCSTR)pvProductVersion);
    return productVersion;
#else
    return zeno::format("{:04d}.{:02d}.{:02d}", year, month, day);
#endif
}

void verifyVersion()
{
    auto ver = getZenoVersion();
    const char *plat =
#if defined(Q_OS_WIN)
                   "windows"
#elif defined(Q_OS_LINUX)
                   "linux"
#else
                   "unknown"
#endif
#if defined(NDEBUG)
                   " release"
#else
                   " debug"
#endif
                   ;
    const char *feat =
#if defined(__INTEL_COMPILER)
                   "icc"
#elif defined(__clang__)
                   "clang"
#elif defined(__GNUC__)
                   "gcc"
#elif defined(_MSC_VER)
                   "msvc"
#else
                   "unknown"
#endif
#if defined(ZENO_MULTIPROCESS)
#if defined(ZENO_IPC_USE_TCP)
                   "+tcp"
#else
                   "+pipe"
#endif
#endif
#if defined(_OPENMP)
                   "+omp"
#endif
#if defined(ZENO_PARALLEL_STL)
                   "+pstl"
#endif
#if defined(ZENO_ENABLE_BACKWARD)
                   "+bt"
#endif
#if defined(ZENO_BENCHMARKING)
                   "+tm"
#endif
                   ;
    // TODO: luzh, may check the internet latest version and compare, if not latest hint the user to update..
    zeno::log_info("zeno {} {} {} {}", plat, ver, __TIME__, feat);
}

int invoke_main(int argc, char *argv[]);
int invoke_main(int argc, char *argv[]) {
    if (argc < 1) {
        zeno::log_error("no enough arguments to -invoke");
        return -1;
    }
    std::string prog = argv[0];
    std::vector<char *> newArgv(argv, argv + argc);
    std::string newProg = QCoreApplication::applicationFilePath().toStdString() + "\0";
    newArgv[0] = newProg.data();
    auto &ud = zeno::getSession().userData();
    if (!ud.has("subprogram_" + prog)) {
        zeno::log_error("no such sub-program named [{}]", prog);
        return -1;
    }
    zeno::log_info("launching sub-program [{}]", prog);
    auto alterMain = ud.get<zeno::GenericObject<int(*)(int, char **)>>("subprogram_" + prog)->get();
    return alterMain(newArgv.size(), newArgv.data());
}

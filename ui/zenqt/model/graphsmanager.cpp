#include "graphsmanager.h"
#include "model/graphstreemodel.h"
#include "model/assetsmodel.h"
#include "uicommon.h"
#include <zeno/io/zsg2reader.h>
#include <zeno/utils/log.h>
#include <zeno/utils/scope_exit.h>
#include <zeno/io/iohelper.h>
#include "util/uihelper.h"
#include <zeno/io/zenwriter.h>
#include <zeno/io/zenreader.h>
#include <zeno/core/Session.h>
#include "nodeeditor/gv/zenosubgraphscene.h"
#include "zassert.h"
#include "variantptr.h"
#include "model/parammodel.h"
#include "zenoapplication.h"
#include "zenomainwindow.h"


GraphsTotalView::GraphsTotalView()
    : _graphMgr(zenoApp->graphsManager())
{
    connect(_graphMgr, &GraphsManager::modelInited, this, &GraphsTotalView::modelInited);
    connect(_graphMgr, &GraphsManager::modelDataChanged, this, &GraphsTotalView::modelDataChanged);
    connect(_graphMgr, &GraphsManager::fileOpened, this, &GraphsTotalView::fileOpened);
    connect(_graphMgr, &GraphsManager::fileClosed, this, &GraphsTotalView::fileClosed);
    connect(_graphMgr, &GraphsManager::fileSaved, this, &GraphsTotalView::fileSaved);
    connect(_graphMgr, &GraphsManager::dirtyChanged, this, &GraphsTotalView::dirtyChanged);
}

GraphsTotalView::GraphsTotalView(const GraphsTotalView& graphsview) {
    _graphMgr = graphsview._graphMgr;
}

void GraphsTotalView::setGraphsMgr(GraphsManager* graph) {
    _graphMgr = zenoApp->graphsManager();
}

void GraphsTotalView::newFile() {
    zenoApp->getMainWindow()->onNewFile();
}

void GraphsTotalView::openFile() {
    zenoApp->getMainWindow()->openFileDialog();
}


GraphsManager::GraphsManager(QObject* parent)
    : QObject(parent)
    , m_model(nullptr)
    , m_logModel(nullptr)
    , m_assets(nullptr)
    , m_main(nullptr)
    , m_version(zeno::VER_3)
    , m_bIniting(false)
    , m_bImporting(false)
{
    m_logModel = new QStandardItemModel(this);
    m_model = new GraphsTreeModel(this);
    //m_main = new GraphModel("/main", false, m_model, this);
    //m_model->init(m_main);
    m_assets = new AssetsModel(this);

}

GraphsManager::GraphsManager(const GraphsManager& rhs) {
    assert(false);
}

GraphsManager::~GraphsManager()
{
}

void GraphsManager::initRootObjects() {
    QQmlApplicationEngine* engine = zenoApp->getQmlEngine();
    engine->rootContext()->setContextProperty("graphsmanager", this);
    if (m_main)
        engine->rootContext()->setContextProperty("nodesModel", m_main);
    if (m_model)
        engine->rootContext()->setContextProperty("treeModel", m_model);
}

void GraphsManager::registerCoreNotify() {

}

AssetsModel* GraphsManager::assetsModel() const
{
    return m_assets;
}

QStandardItemModel* GraphsManager::logModel() const
{
    return m_logModel;
}

GraphModel* GraphsManager::getGraph(const QStringList& objPath) const
{
    if (objPath.empty())
        return nullptr;

    if (objPath[0] == "main") {
        return m_model ? m_model->getGraphByPath(objPath) : nullptr;
    }
    else {
        QStringList assetGraphPath = objPath;
        GraphModel* pModel = m_assets->getAssetGraph(assetGraphPath[0]);
        if (!pModel) return nullptr;

        assetGraphPath.removeAt(0);
        if (assetGraphPath.isEmpty())
            return pModel;

        return pModel->getGraphByPath(assetGraphPath);
    }
}

GraphsTreeModel* GraphsManager::openZsgFile(const QString& fn, zenoio::ERR_CODE& code)
{
    zeno::ZSG_VERSION ver = zenoio::getVersion(fn.toStdString());
    zenoio::ZSG_PARSE_RESULT result;

    m_bIniting = true;
    zeno::scope_exit sp([=] { m_bIniting = false; });

    if (ver == zeno::VER_2_5) {
        zenoio::Zsg2Reader reader;
        result = reader.openFile(fn.toStdString());
    }
    else if (ver == zeno::VER_3) {
        zenoio::ZenReader reader;
        result = reader.openFile(fn.toStdString());
    }
    else {
        m_version = zeno::UNKNOWN_VER;
        result.code = zenoio::PARSE_VERSION_UNKNOWN;
    }

    if (result.code != zenoio::PARSE_NOERROR)
    {
        code = result.code;
        return nullptr;
    }

    m_version = ver;

    m_timerInfo = result.timeline;
    createGraphs(result);
    //reset model.
    newFile();
    m_filePath = fn;

    emit fileOpened(fn);
    m_model->markDirty(false);
    return m_model;
}

bool GraphsManager::isInitializing() const
{
    return m_bIniting;
}

bool GraphsManager::isImporting() const
{
    return m_bImporting;
}

void GraphsManager::createGraphs(const zenoio::ZSG_PARSE_RESULT ioresult)
{
    ZASSERT_EXIT(m_assets);
    zeno::getSession().initEnv(ioresult);
}

bool GraphsManager::saveFile(const QString& filePath, APP_SETTINGS)
{
    if (m_model == nullptr) {
        zeno::log_error("The current model is empty.");
        return false;
    }

    zenoio::AppSettings settings;       //TODO:
    settings.timeline = zenoApp->getMainWindow()->timelineInfo();

    zeno::GraphData graph = zeno::getSession().mainGraph->exportGraph();

    zenoio::ZenWriter writer;
    std::string strContent = writer.dumpProgramStr(graph, settings);
    QFile f(filePath);
    zeno::log_debug("saving {} chars to file [{}]", strContent.size(), filePath.toStdString());
    if (!f.open(QIODevice::WriteOnly)) {
        qWarning() << Q_FUNC_INFO << "Failed to open" << filePath << f.errorString();
        zeno::log_error("Failed to open file for write: {} ({})", filePath.toStdString(),
                        f.errorString().toStdString());
        return false;
    }

    f.write(strContent.c_str());
    f.close();
    zeno::log_info("saved '{}' successfully", filePath.toStdString());

    m_filePath = filePath;

    m_model->clearDirty();

    QFileInfo info(filePath);
    emit fileSaved(filePath);
    return true;
}

GraphsTreeModel* GraphsManager::newFile()
{
    clear();
    m_main = new GraphModel("/main", false, m_model, this);
    m_model->init(m_main);

    //TODO: assets may be kept.
    initRootObjects();

    emit modelInited();

    connect(m_model, SIGNAL(rowsAboutToBeRemoved(const QModelIndex&, int, int)),
        this, SLOT(onRowsAboutToBeRemoved(const QModelIndex&, int, int)));
    connect(m_model, &GraphsTreeModel::dirtyChanged, this, [=]() {
        emit dirtyChanged(m_model->isDirty());
    });

    return m_model;
}

void GraphsManager::importGraph(const QString& fn)
{
    m_bImporting = true;
    zeno::scope_exit sp([=] { m_bImporting = false; });
    //todo: the function needs to be refactor.
}

void GraphsManager::importSubGraphs(const QString& fn, const QMap<QString, QString>& map)
{
    m_bImporting = true;
    zeno::scope_exit sp([=] { m_bImporting = false; });
    //todo: the function needs to be refactor.
}

void GraphsManager::clear()
{
    if (m_model)
    {
        m_model->clear();

        for (auto scene : m_scenes)
        {
            delete scene;
        }
        m_scenes.clear();
    }

    //clear main model
    if (m_main) {
        m_main->clear();
        delete m_main;
        m_main = nullptr;
        zeno::getSession().resetMainGraph();
    }
    m_filePath = "";
    emit fileClosed();
}

void GraphsManager::onRowsAboutToBeRemoved(const QModelIndex& parent, int first, int last)
{
    //TODO: deprecated.
    /*
    const QModelIndex& idx = m_model->index(first, 0);
    if (idx.isValid())
    {
        const QString& subgName = idx.data(ROLE_CLASS_NAME).toString();
        if (m_scenes.find(subgName) != m_scenes.end())
        {
            delete m_scenes[subgName];
            m_scenes.remove(subgName);
        }
    }
    */
}

void GraphsManager::onModelDataChanged(const QModelIndex& subGpIdx, const QModelIndex& idx, int role)
{
    switch (role)
    {
    case ROLE_OBJPOS:
    case ROLE_COLLASPED:
        break;
    default:
        emit modelDataChanged();
        break;
    }
}

void GraphsManager::removeCurrent()
{
    if (m_model) {
        
    }
}

QGraphicsScene* GraphsManager::gvScene(const QStringList& graphPath) const
{
    for (auto scene : m_scenes) {
        auto pModel = scene->getGraphModel();
        auto path = pModel->currentPath();
        if (path == graphPath)
            return scene;
    }
    return nullptr;
}

QGraphicsScene* GraphsManager::gvScene(const QModelIndex& subgIdx) const
{
    return nullptr;
    /*
    if (!subgIdx.isValid())
        return nullptr;

    const QString& subgName = subgIdx.data(ROLE_CLASS_NAME).toString();
    if (m_scenes.find(subgName) == m_scenes.end())
        return nullptr;

    return m_scenes[subgName];
    */
}

QMap<QStringList, QGraphicsScene*> GraphsManager::gvSubScenes(const QStringList& graphName) const
{
    QMap<QStringList, QGraphicsScene*> pathSceneMap;
    QString graphPath = graphName.join("");
    for (auto scene : m_scenes) {
        auto pModel = scene->getGraphModel();
        auto path = pModel->currentPath();
        if (path.join("").contains(graphPath)) {
            pathSceneMap.insert(path, scene);
        }
    }
    return pathSceneMap;
}

void GraphsManager::addScene(const QModelIndex& subgIdx, ZenoSubGraphScene* scene)
{
    //TODO: deprecated
    /*
    const QString& subgName = subgIdx.data(ROLE_CLASS_NAME).toString();
    if (m_scenes.find(subgName) != m_scenes.end() || !scene)
        return;
    m_scenes.insert(subgName, scene);
    */
}

void GraphsManager::addScene(const QStringList& graphPath, ZenoSubGraphScene* scene)
{
    for (auto scene : m_scenes) {
        auto path = scene->getGraphModel()->currentPath();
        if (path == graphPath)
            return;
    }
    m_scenes.push_back(scene);
}

bool GraphsManager::removeScene(const QStringList& graphPath)
{
    for (int i = 0; i < m_scenes.size(); i++) {
        auto path = m_scenes[i]->getGraphModel()->currentPath();
        if (path == graphPath)
        {
            delete m_scenes[i];
            m_scenes.removeAt(i);
            return true;
        }
    }
    return false;
}

zeno::TimelineInfo GraphsManager::timeInfo() const
{
    return m_timerInfo;
}

QString GraphsManager::zsgPath() const
{
    return m_filePath;
}

QString GraphsManager::zsgDir() const
{
    const QString& zsgpath = zsgPath();
    QFileInfo fp(zsgpath);
    return fp.absolutePath();
}

USERDATA_SETTING GraphsManager::userdataInfo() const
{
    //TODO
    return USERDATA_SETTING();
}

RECORD_SETTING GraphsManager::recordSettings() const
{
    return RECORD_SETTING();
}

zeno::NodeCates GraphsManager::getCates() const
{
    zeno::NodeCates cates = zeno::getSession().dumpCoreCates();
    std::vector<std::string> assetsNames;

    for (int r = 0; r < m_assets->rowCount(); r++)
    {
        QModelIndex idx = m_assets->index(r);
        const QString& asset = idx.data(ROLE_CLASS_NAME).toString();
        assetsNames.push_back(asset.toStdString());
    }

    cates.insert(std::make_pair("assets", assetsNames));
    return cates;
}

void GraphsManager::updateAssets(const QString& assetsName, zeno::ParamsUpdateInfo info, const zeno::CustomUI& customui)
{
    zeno::getSession().assets->updateAssets(assetsName.toStdString(), info, customui);
    //update to each assets node on the tree
    GraphModel* mainM = m_model->getGraphByPath({"main"});
    ZASSERT_EXIT(mainM);
    mainM->syncToAssetsInstance(assetsName, info, customui);

    //also need to sync all other assets.
    for (int i = 0; i < m_assets->rowCount(); i++)
    {
        GraphModel* pAssetM = m_assets->getAssetGraph(i);
        if (pAssetM && pAssetM->name() != assetsName)
        {
            pAssetM->syncToAssetsInstance(assetsName, info, customui);
        }
    }
}

zeno::ZSG_VERSION GraphsManager::ioVersion() const
{
    return m_version;
}

void GraphsManager::setIOVersion(zeno::ZSG_VERSION ver)
{
    m_version = ver;
}

void GraphsManager::clearMarkOnGv() {
    for (auto scene : m_scenes) {
        if (scene->getGraphModel()->currentPath().startsWith("main")) {
            scene->clearMark();
        }
    }
}

void GraphsManager::appendErr(const QString& nodeName, const QString& msg)
{
    if (msg.trimmed().isEmpty())
        return;

    QStandardItem* item = new QStandardItem(msg);
    item->setData(QtFatalMsg, ROLE_LOGTYPE);
    item->setData(nodeName, ROLE_NODE_IDENT);
    item->setEditable(false);
    item->setData(QBrush(QColor(200, 84, 79)), Qt::ForegroundRole);
    m_logModel->appendRow(item);
}

void GraphsManager::appendLog(QtMsgType type, QString fileName, int ln, const QString &msg)
{
    if (msg.trimmed().isEmpty())
        return;

    QStandardItem *item = new QStandardItem(msg);
    item->setData(type, ROLE_LOGTYPE);
    item->setData(fileName, ROLE_FILENAME);
    item->setData(ln, ROLE_LINENO);
    item->setEditable(false);
    switch (type)
    {
        //todo: time
        case QtDebugMsg:
        {
            item->setData(QBrush(QColor(200, 200, 200, 0.7 * 255)), Qt::ForegroundRole);
            m_logModel->appendRow(item);
            break;
        }
        case QtCriticalMsg:
        {
            item->setData(QBrush(QColor(80, 154, 200)), Qt::ForegroundRole);
            m_logModel->appendRow(item);
            break;
        }
        case QtInfoMsg:
        {
            item->setData(QBrush(QColor(51, 148, 85)), Qt::ForegroundRole);
            m_logModel->appendRow(item);
            break;
        }
        case QtWarningMsg:
        {
            item->setData(QBrush(QColor(200, 154, 80)), Qt::ForegroundRole);
            m_logModel->appendRow(item);
            break;
        }
        case QtFatalMsg:
        {
            item->setData(QBrush(QColor(200, 84, 79)), Qt::ForegroundRole);
            m_logModel->appendRow(item);
            break;
        }
    default:
        delete item;
        break;
    }
}

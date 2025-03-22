#include "calculationmgr.h"
#include <zeno/core/Session.h>
#include <zeno/extra/GraphException.h>
#include "viewport/displaywidget.h"
#include "zassert.h"
#include "util/uihelper.h"
#include "zenoapplication.h"
#include "zenomainwindow.h"
#include <zeno/core/common.h>
#include "model/graphsmanager.h"
#include "model/GraphsTreeModel.h"
#include "widgets/ztimeline.h"
#include "declmetatype.h"


CalcWorker::CalcWorker(QObject* parent) {
    auto& sess = zeno::getSession();
    if (m_bReportNodeStatus) {
        sess.registerNodeCallback([=](zeno::ObjPath nodePath, bool bDirty, zeno::NodeRunStatus status) {
            NodeState state;
            state.bDirty = bDirty;
            state.runstatus = status;
            emit nodeStatusChanged(nodePath, state);
            });
    }
}

void CalcWorker::setCurrentGraphPath(const QString& current_graph_path) {
    m_current_graph_path = current_graph_path;
}

void CalcWorker::run() {
    auto& sess = zeno::getSession();

    //sess.registerCommitRender([&](zeno::render_update_info info) {
    //    emit commitRenderInfo(info);
    //});

    zeno::GraphException::catched([&] {
        const std::string& currpath = m_current_graph_path.toStdString();
        sess.run(/*currpath*/); //如果局部运行子图，可能要考虑subinput的外部前置计算是否要计算
        }, *sess.globalError);
    sess.globalState->set_working(false);

    if (sess.globalError->failed()) {
        QString errMsg = QString::fromStdString(sess.globalError->getErrorMsg());
        NodeState state;
        state.bDirty = true;
        state.runstatus = zeno::Node_RunError;
        zeno::ObjPath path = sess.globalError->getNode();
        emit nodeStatusChanged(path, state);
        emit calcFinished(false, path, errMsg); //会发送到：DisplayWidget::onCalcFinished
    }
    else {
        emit calcFinished(true, {}, "");
    }
}


CalculationMgr::CalculationMgr(QObject* parent)
    : QObject(parent)
    , m_bMultiThread(false)
    , m_worker(nullptr)
    , m_playTimer(new QTimer(this))
    , m_runstatus(RunStatus::NoRun)
{
    m_worker = new CalcWorker(this);
    m_worker->moveToThread(&m_thread);
    connect(&m_thread, &QThread::started, m_worker, &CalcWorker::run);
    connect(m_worker, &CalcWorker::calcFinished, this, &CalculationMgr::onCalcFinished);
    connect(m_worker, &CalcWorker::commitRenderInfo, this, &CalculationMgr::commitRenderInfo) ;
    connect(m_worker, &CalcWorker::nodeStatusChanged, this, &CalculationMgr::onNodeStatusReported);
    connect(m_playTimer, SIGNAL(timeout()), this, SLOT(onPlayReady()));

    auto& sess = zeno::getSession();
    sess.registerRunTrigger([=]() {
        run();
    });
}

void CalculationMgr::onNodeStatusReported(zeno::ObjPath uuidPath, NodeState state)
{
    GraphsTreeModel* pMainTree = zenoApp->graphsManager()->currentModel();
    if (pMainTree) {
        const QModelIndex targetNode = pMainTree->getIndexByUuidPath(uuidPath);
        if (targetNode.isValid()) {
            UiHelper::qIndexSetData(targetNode, QVariant::fromValue(state), QtRole::ROLE_NODE_RUN_STATE);
            if (!m_bMultiThread) {
                //TODO: 处理的时间里可能会包括改变节点状态和数据的操作，比如滑动时间轴，所以必须要控制事件的范围
                //zenoApp->processEvents();
            }
        }
    }
}

void CalculationMgr::onCalcFinished(bool bSucceed, zeno::ObjPath nodeUuidPath, QString msg)
{
    //确保此时计算线程不再跑逻辑，这里暂时是代码上约束，也就是CalcWorker::run()走完就发信号。
    if (m_bMultiThread)
    {
        m_thread.quit();
        m_thread.wait();
    }
    setRunStatus(RunStatus::NoRun);
    emit calcFinished(bSucceed, nodeUuidPath, msg);  //会发送到：DisplayWidget::onCalcFinished
}

void CalculationMgr::run()
{
    setRunStatus(RunStatus::Running);
    m_worker->setCurrentGraphPath(zenoApp->graphsManager()->currentGraphPath());
    if (m_bMultiThread) {
        m_thread.start();
    }
    else {
        m_worker->run();
    }
}

void CalculationMgr::onPlayReady() {
    auto& sess = zeno::getSession();
    if (!sess.is_auto_run()) {
        run();
    }
    //切到下一帧
    int frame = sess.globalState->getFrameId();
    sess.switchToFrame(frame + 1);

    //ui上也同步这一帧
    if (auto mainWin = zenoApp->getMainWindow()) {
        if (auto timeline = mainWin->timeline()) {
            timeline->blockSignals(true);
            timeline->setSliderValue(frame + 1);
            timeline->blockSignals(false);
        }
    }

    m_playTimer->start();
}

void CalculationMgr::onPlayTriggered(bool bToggled) {
    if (m_playTimer) {
        if (bToggled) {
            m_playTimer->start();
        }
        else {
            m_playTimer->stop();
        }

        if (auto mainWin = zenoApp->getMainWindow()) {
            if (auto timeline = mainWin->timeline()) {
                bool block = timeline->signalsBlocked();
                timeline->blockSignals(true);
                timeline->setPlayButtonChecked(bToggled);
                timeline->blockSignals(block);
            }
        }
    }
}

void CalculationMgr::onFrameSwitched(int frame) {
    //手动移动时间轴

    //停止播放
    m_playTimer->stop();
    if (auto mainWin = zenoApp->getMainWindow()) {
        if (auto timeline = mainWin->timeline()) {
            bool block = timeline->signalsBlocked();
            timeline->blockSignals(true);
            timeline->setPlayButtonChecked(false);
            timeline->blockSignals(block);
        }
    }

    auto& sess = zeno::getSession();
    sess.switchToFrame(frame);
}

void CalculationMgr::kill()
{
    zeno::getSession().interrupt();
    zeno::getSession().globalState->set_working(false);
}

void CalculationMgr::clear()
{
    zeno::getSession().set_Rerun();
    for (auto view : zenoApp->getMainWindow()->viewports())
        view->cleanUpScene();
}

RunStatus::Value CalculationMgr::getRunStatus() const
{
    return m_runstatus;
}

void CalculationMgr::setRunStatus(RunStatus::Value runstatus)
{
    if (m_runstatus == runstatus) {
        return;
    }
    m_runstatus = runstatus;
    emit runStatus_changed();
}

bool CalculationMgr::getAutoRun() const
{
    auto& sess = zeno::getSession();
    return sess.is_auto_run();
}

void CalculationMgr::setAutoRun(bool autoRun)
{
    if (getAutoRun() == autoRun)
        return;
    auto& sess = zeno::getSession();
    sess.set_auto_run(autoRun);
    emit autorun_changed();
}

void CalculationMgr::registerRenderWid(DisplayWidget* pDisp)
{
    m_registerRenders.insert(pDisp);
    connect(this, &CalculationMgr::calcFinished, pDisp, &DisplayWidget::onCalcFinished);
    connect(this, &CalculationMgr::commitRenderInfo, pDisp,
        &DisplayWidget::onRenderInfoCommitted);
    connect(pDisp, &DisplayWidget::render_objects_loaded, this, &CalculationMgr::on_render_objects_loaded);
}

void CalculationMgr::unRegisterRenderWid(DisplayWidget* pDisp) {
    m_loadedRender.remove(pDisp);
}

bool CalculationMgr::isMultiThreadRunning() const {
    return m_bMultiThread;
}

void CalculationMgr::on_render_objects_loaded()
{
    DisplayWidget* pWid = qobject_cast<DisplayWidget*>(sender());
    ZASSERT_EXIT(pWid);
    m_loadedRender.insert(pWid);
    if (m_loadedRender.size() == m_registerRenders.size())
    {
        //todo: notify calc to continue, if still have something to calculate.
    }
}

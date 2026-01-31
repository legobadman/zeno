#include <zeno/core/Session.h>
#include <iobject2.h>
#include <zeno/core/INodeClass.h>
#include <zeno/core/Assets.h>
#include <zeno/extra/GlobalState.h>
#include <zeno/extra/GlobalComm.h>
#include <zeno/extra/GlobalError.h>
#include <zeno/extra/EventCallbacks.h>
#include <zeno/extra/PyExecuteProxy.h>
#include <zeno/types/UserData.h>
#include <zeno/core/Graph.h>
#include <zeno/core/NodeImpl.h>
#include <zeno/core/SolverImpl.h>
#include <zeno/core/CoreParam.h>
#include <zeno/utils/safe_at.h>
#include <zeno/utils/logger.h>
#include <zeno/utils/string.h>
#include <zeno/utils/helper.h>
#include <zeno/zeno.h>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include <zeno/extra/SubnetNode.h>
#include <zeno/extra/GraphException.h>
#include <zeno/core/GlobalVariable.h>
#include <zeno/core/FunctionManager.h>
#include <regex>
#include <Windows.h>
#include <zeno/extra/CalcContext.h>
#include <zeno/core/ObjectRecorder.h>
#include <reflect/core.hpp>
#include <reflect/type.hpp>
#include <reflect/metadata.hpp>
#include <reflect/registry.hpp>
#include <reflect/container/any>
#include <reflect/container/arraylist>
#ifdef ZENO_WITH_PYTHON
#include <Python.h>
#endif
#ifdef _WIN32
#include <Windows.h>
#endif
#include <zeno/core/reflectdef.h>
#include "zeno_types/reflect/reflection.generated.hpp"


using namespace zeno::reflect;
using namespace zeno::types;

namespace zeno {

static Session* s_ptrGlobal = nullptr;

ZENO_API Session::Session()
    : globalState(std::make_unique<GlobalState>())
    , globalComm(std::make_unique<GlobalComm>())
    , globalError(std::make_unique<GlobalError>())
    , eventCallbacks(std::make_unique<EventCallbacks>())
    , m_userData(std::make_unique<UserData>())
    //, m_spMainGraph(std::make_shared<Graph>("main"))
    , assets(std::make_unique<AssetsMgr>())
    , m_pyexecutor(std::make_unique<PyExecuteProxy>())
    , globalVariableManager(std::make_unique<GlobalVariableManager>())
    , funcManager(std::make_unique<FunctionManager>())
    , m_recorder(std::make_unique<ObjectRecorder>())
    , m_mainThreadId(0)
{
    m_mainThreadId = GetCurrentThreadId();
}

ZENO_API Session::~Session() {
    s_ptrGlobal = nullptr;
}

void Session::destroy() {
    assets.reset();
    m_spMainGraph.reset();
}


static CustomUI descToCustomui(const Descriptor& desc) {
    //兼容以前写的各种ZENDEFINE
    CustomUI ui;

    ui.nickname = desc.displayName;
    ui.uistyle.iconResPath = desc.iconResPath;
    ui.doc = desc.doc;
    if (!desc.categories.empty())
        ui.category = desc.categories[0];   //很多cate都只有一个

    ParamGroup default_group;
    for (const SocketDescriptor& param_desc : desc.inputs) {
        if (param_desc._desc.type == Desc_Prim) {
            //有可能没赋初值，要检查一下
            if (!param_desc._desc.primparam.defl.has_value()) {
                ParamPrimitive newparam = param_desc._desc.primparam;
                newparam.defl = initAnyDeflValue(newparam.type);
                default_group.params.push_back(newparam);
            }
            else {
                //字符串的defl都是以const char*作为内部储存的类型，这里转一下string，以免上层逻辑处理疏漏
                ParamPrimitive newparam = param_desc._desc.primparam;
                if (newparam.defl.type() == zeno::reflect::type_info<const char*>()) {
                    std::string str = zeno::reflect::any_cast<const char*>(newparam.defl);
                    newparam.defl = str;
                }
                convertToEditVar(newparam.defl, newparam.type);
                default_group.params.push_back(newparam);
            }
        }
        else if (param_desc._desc.type == Desc_Obj) {
            ui.inputObjs.push_back(param_desc._desc.objparam);
        }
        else {
            ParamType type = param_desc.type;
            if (isPrimitiveType(type)) {
                //如果是数值类型，就添加到组里
                ParamPrimitive param;
                param.name = param_desc.name;
                param.type = type;
                param.defl = zeno::str2any(param_desc.defl, param.type);
                convertToEditVar(param.defl, param.type);
                if (param_desc.socketType != zeno::NoSocket)
                    param.socketType = param_desc.socketType;
                if (param_desc.control != NullControl)
                    param.control = param_desc.control;
                if (!param_desc.comboxitems.empty()) {
                    //compatible with old case of combobox items.
                    param.type = zeno::types::gParamType_String;
                    param.control = Combobox;
                    std::vector<std::string> items = split_str(param_desc.comboxitems, ' ', false);
                    if (!items.empty()) {
                        items.erase(items.begin());
                        param.ctrlProps = items;
                    }
                }
                if (param.type != Param_Null && param.control == NullControl)
                    param.control = getDefaultControl(param.type);
                param.tooltip = param_desc.doc;
                param.sockProp = Socket_Normal;
                param.wildCardGroup = param_desc.wildCard;
                param.bSocketVisible = false;
                default_group.params.push_back(param);
            }
            else
            {
                //其他一律认为是对象（Zeno目前的类型管理非常混乱，有些类型值是空字符串，但绝大多数是对象类型
                ParamObject param;
                param.name = param_desc.name;
                param.type = type;
                param.socketType = Socket_Clone;        //在此版本里，不再区分owing, readonly clone，全都是clone.
                //if (param_desc.socketType != zeno::NoSocket)
                //    param.socketType = param_desc.socketType;
                param.bInput = true;
                param.wildCardGroup = param_desc.wildCard;

                //dict和list允许多连接口，且不限定对象类型（但只能是对象，暂不接收primitive，否则就违反了对象和primitive分开连的设计了）
                if (type == gParamType_List) {
                    param.sockProp = Socket_MultiInput;
                }
                else {
                    param.sockProp = Socket_Normal;
                }

                ui.inputObjs.push_back(param);
            }
        }
    }
    for (const ParamDescriptor& param_desc : desc.params) {
        ParamPrimitive param;
        param.name = param_desc.name;
        param.type = param_desc.type;
        param.defl = zeno::str2any(param_desc.defl, param.type);
        convertToEditVar(param.defl, param.type);
        param.socketType = NoSocket;
        //其他控件估计是根据类型推断的。
        if (!param_desc.comboxitems.empty()) {
            //compatible with old case of combobox items.
            param.type = zeno::types::gParamType_String;
            param.control = Combobox;
            std::vector<std::string> items = split_str(param_desc.comboxitems, ' ', false);
            if (!items.empty()) {
                items.erase(items.begin());
                param.ctrlProps = items;
            }
        }
        if (param.type != Param_Null) {
            if (param_desc.control == NullControl) {
                param.control = getDefaultControl(param.type);
            } else {
                param.control = param_desc.control;
            }
        }
        param.tooltip = param_desc.doc;
        param.bSocketVisible = false;
        default_group.params.push_back(param);
    }
    for (const SocketDescriptor& param_desc : desc.outputs) {
        if (param_desc._desc.type == Desc_Prim) {
            ui.outputPrims.push_back(param_desc._desc.primparam);
        }
        else if (param_desc._desc.type == Desc_Obj) {
            ui.outputObjs.push_back(param_desc._desc.objparam);
        }
        else {
            ParamType type = param_desc.type;
            if (isPrimitiveType(type)) {
                //如果是数值类型，就添加到组里
                ParamPrimitive param;
                param.name = param_desc.name;
                param.type = type;
                param.defl = zeno::str2any(param_desc.defl, param.type);
                //输出的数据端口没必要将vec转为vecedit
                if (param_desc.socketType != zeno::NoSocket)
                    param.socketType = param_desc.socketType;
                param.control = NullControl;
                param.tooltip = param_desc.doc;
                param.sockProp = Socket_Normal;
                param.wildCardGroup = param_desc.wildCard;
                param.bSocketVisible = false;
                ui.outputPrims.push_back(param);
            }
            else
            {
                //其他一律认为是对象（Zeno目前的类型管理非常混乱，有些类型值是空字符串，但绝大多数是对象类型
                ParamObject param;
                param.name = param_desc.name;
                param.type = type;
                if (param_desc.socketType != zeno::NoSocket)
                    param.socketType = param_desc.socketType;
                if (!param.bWildcard)  //输出可能是wildCard
                    param.socketType = Socket_Output;
                param.bInput = false;
                param.sockProp = Socket_Normal;
                param.wildCardGroup = param_desc.wildCard;
                ui.outputObjs.push_back(param);
            }
        }
    }
    ParamTab tab;
    tab.groups.emplace_back(std::move(default_group));
    ui.inputPrims.emplace_back(std::move(tab));
    return ui;
}

ZENO_API INodeClass::INodeClass(CustomUI const &customui, std::string const& classname)
    : m_customui(customui)
    , classname(classname)
{
}

ZENO_API INodeClass::~INodeClass() {
    if (classname == "erode_noise_perlin_GEO") {
        int j;
        j = 0;
    }
}

ZENO_API std::shared_ptr<Graph> Session::createGraph(const std::string& name) {
    auto graph = std::make_shared<Graph>(name);
    return graph;
}

ZENO_API std::shared_ptr<Graph> Session::mainGraph() const {
    return m_spMainGraph;
}

ZENO_API NodeImpl* Session::getNodeByUuidPath(std::string const& uuid_path) {
    return m_spMainGraph->getNodeByUuidPath(uuid_path);
}

ZENO_API NodeImpl* Session::getNodeByPath(std::string const& uuid_path)
{
    return m_spMainGraph->getNodeByPath(uuid_path);
}

ZENO_API void Session::resetMainGraph() {
    m_spMainGraph.reset();
    m_spMainGraph = std::make_shared<Graph>("main");
    globalVariableManager.reset();
    globalVariableManager = std::make_unique<GlobalVariableManager>();
}

void Session::clearMainGraph() {
    m_spMainGraph.reset();
}

ZENO_API void Session::clearAssets() {
    assets->clear();
}

ZENO_API void Session::setApiLevelEnable(bool bEnable)
{
    m_bApiLevelEnable = bEnable;
}

ZENO_API void Session::beginApiCall()
{
    if (!m_bApiLevelEnable || m_bDisableRunning) return;
    m_apiLevel++;
}

ZENO_API void Session::endApiCall()
{
    if (!m_bApiLevelEnable || m_bDisableRunning) return;
    m_apiLevel--;
    if (m_apiLevel == 0) {
        if (m_bAutoRun) {
            if (m_callbackRunTrigger) {
                m_callbackRunTrigger();
            }
            else {
                //这里后续是给非ui框架使用，比如命令行，而ui会走RunTrigger.
                zeno::render_reload_info infos;
                run("", infos);
            }
        }
    }
}

ZENO_API void Session::setDisableRunning(bool bOn)
{
    m_bDisableRunning = bOn;
}

ZENO_API void Session::registerRunTrigger(std::function<void()> func)
{
    m_callbackRunTrigger = func;
}

ZENO_API void Session::registerNodeCallback(F_NodeStatus func)
{
    m_funcNodeStatus = func;
}

void Session::registerIOCallback(F_IOProgress func) {
    m_funcIOCallback = func;
}

ZENO_API std::shared_ptr<Graph> Session::getGraphByPath(const std::string& path) {
    //对于assets
    //可能的形式包括： /ABC/subnet1/subnet2   ABC   /ABC
    std::vector<std::string> items = split_str(path, '/', false);
    if (items.empty()) {
        return nullptr;
    }

    std::string graph_name = items[0];
    if (graph_name == "main" && m_spMainGraph) {
        return m_spMainGraph->getGraphByPath(path);
    }
    else if (auto spGraph = assets->getAssetGraph(graph_name, true)) {
        return spGraph->getGraphByPath(path);
    }
    return nullptr;
}

void Session::reportIOProgress(const std::string& info, int inc) {
    if (m_funcIOCallback) {
        m_funcIOCallback(info, inc);
    }
}

void Session::reportNodeStatus(const ObjPath& path, bool bDirty, NodeRunStatus status)
{
    if (m_funcNodeStatus) {
        m_funcNodeStatus(path, bDirty, status);
    }
}

ZENO_API void Session::switchToFrame(int frameid)
{
    CORE_API_BATCH
    if (m_spMainGraph) {
        m_spMainGraph->markDirtyWhenFrameChanged();
    }
    if (globalState) {
        globalState->updateFrameId(frameid);
    }
}

ZENO_API void Session::updateFrameRange(int start, int end)
{
    CORE_API_BATCH
    globalState->updateFrameRange(start, end);
}

ZENO_API void Session::interrupt() {
    m_bInterrupted = true;
}

bool Session::is_async_executing() const {
    return m_bAsyncExecute;
}

void Session::set_async_executing(bool bOn) {
    m_bAsyncExecute = bOn;
}

ZENO_API unsigned long Session::mainThreadId() const {
    return m_mainThreadId;
}

ZENO_API void Session::setMainThreadId(unsigned long threadId) {
    m_mainThreadId = threadId;
}

ZENO_API void Session::set_solver(const std::string& solver) {
    m_solver = solver;
}

ZENO_API std::string Session::get_solver() {
    return m_solver;
}

std::wstring Session::get_project_path() const {
    return m_proj_path;
}

void Session::init_project_path(const std::wstring& path) {
    m_proj_path = path;
}

void Session::terminate_solve() {
    SolverImpl* pSolverNode = static_cast<SolverImpl*>(getNodeByUuidPath(m_solver));
    if (pSolverNode) {
        pSolverNode->terminate_solve();
    }
}

ZENO_API bool Session::run(const std::string& currgraph, render_reload_info& infos) {
    if (m_bDisableRunning)
        return false;

    if (m_bReentrance) {
        return true;
    }

    m_bReentrance = true;
    m_bInterrupted = false;
    globalState->set_working(true);

    zeno::scope_exit sp([&]() { 
        m_bReentrance = false;
        m_bInterrupted = false;
        globalState->clearState();
    });

    globalError->clearState();
    float total_time = m_spMainGraph->statistic_cpu_used();
    globalState->init_total_runtime(total_time);

    bool bFailed = false;
    try
    {
        m_spMainGraph->runGraph(infos);
    }
    catch (ErrorException const& e) {
        infos.error.set_node_info(e.get_node_info());
        infos.error.set_error(e.getError());
        bFailed = true;
    }
    catch (std::exception const& e) {
        std::string err = e.what();
        std::string wtf = e.what();
        bFailed = true;
    }
    catch (...) {
        bFailed = true;
    }
    if (!infos.error.failed() && bFailed) {
        m_spMainGraph->mark_clean();
    }
    return true;
}

ZENO_API void Session::set_auto_run(bool bOn) {
    m_bAutoRun = bOn;
}

ZENO_API bool Session::is_auto_run() const {
    return m_bAutoRun;
}

bool Session::is_frame_node(const std::string& node_cls) {
    static std::set<std::string> frame_node_cls = {
        "GetFrameNum",
        "CameraNode",
        "FlipSolver",
        "NewFBXSceneInfo",
        "GetAlembicCamera"
    };
    return frame_node_cls.find(node_cls) != frame_node_cls.end();
}

int Session::get_frame_id() const {
    return globalState->getFrameId();
}

ZENO_API void Session::markDirtyAndCleanResult()
{
    if (m_spMainGraph)
        m_spMainGraph->markDirtyAndCleanup();
}

ZENO_API void Session::initEnv(const zenoio::ZSG_PARSE_RESULT ioresult) {

    bool bDisableRun = m_bDisableRunning;
    m_bDisableRunning = true;
    scope_exit sp([&]() {m_bDisableRunning = bDisableRun; });

    resetMainGraph();
    m_spMainGraph->init(ioresult.mainGraph);
    m_spMainGraph->initRef(ioresult.mainGraph);
    m_proj_path = ioresult.path;
    //referManager->init(m_spMainGraph);

    switchToFrame(ioresult.timeline.currFrame);
    //init $F globalVariable
    //zeno::getSession().globalVariableManager->overrideVariable(zeno::GVariable("$F", zeno::reflect::make_any<float>(ioresult.timeline.currFrame)));
}

void Session::initPyzen(std::function<void()> pyzenFunc) {
    if (m_pyexecutor)
        m_pyexecutor->initPyzenFunc(pyzenFunc);
}

bool Session::asyncRunPython(const std::string& code) {
    if (m_pyexecutor)
        return m_pyexecutor->runPython(code);
    return false;
}

bool Session::runPythonInteractive(const std::string& line, bool& needMore, std::string& output) {
    if (m_pyexecutor)
        return m_pyexecutor->runPythonInteractive(line, needMore, output);
    return false;
}

bool Session::completePython(const std::string& text, std::vector<std::string>& out) {
    if (m_pyexecutor)
        return m_pyexecutor->completePython(text, out);
    return false;
}

void* Session::hEventOfPyFinish() {
    return nullptr;
}

namespace {
std::string dumpDescriptorToJson(const std::string &key, const Descriptor& descriptor) {
    using namespace rapidjson;
    Document doc;
    doc.SetArray();

    // Inputs array
    Value inputs(kArrayType);
    for (const auto& input : descriptor.inputs) {
        Value inputArray(kArrayType);
        inputArray.PushBack(Value().SetString(zeno::paramTypeToString(input.type).c_str(), doc.GetAllocator()), doc.GetAllocator());
        inputArray.PushBack(Value().SetString(input.name.c_str(), doc.GetAllocator()), doc.GetAllocator());
        inputArray.PushBack(Value().SetString(input.defl.c_str(), doc.GetAllocator()), doc.GetAllocator());
        inputArray.PushBack(Value().SetString(input.doc.c_str(), doc.GetAllocator()), doc.GetAllocator());
        inputs.PushBack(inputArray, doc.GetAllocator());
    }

    // Outputs array
    Value outputs(kArrayType);
    for (const auto& output : descriptor.outputs) {
        Value outputArray(kArrayType);
        outputArray.PushBack(Value().SetString(zeno::paramTypeToString(output.type).c_str(), doc.GetAllocator()), doc.GetAllocator());
        outputArray.PushBack(Value().SetString(output.name.c_str(), doc.GetAllocator()), doc.GetAllocator());
        outputArray.PushBack(Value().SetString(output.defl.c_str(), doc.GetAllocator()), doc.GetAllocator());
        outputArray.PushBack(Value().SetString(output.doc.c_str(), doc.GetAllocator()), doc.GetAllocator());
        outputs.PushBack(outputArray, doc.GetAllocator());
    }

    // Params array
    Value params(kArrayType);
    for (const auto& param : descriptor.params) {
        Value paramArray(kArrayType);
        paramArray.PushBack(Value().SetString(zeno::paramTypeToString(param.type).c_str(), doc.GetAllocator()), doc.GetAllocator());
        paramArray.PushBack(Value().SetString(param.name.c_str(), doc.GetAllocator()), doc.GetAllocator());
        paramArray.PushBack(Value().SetString(param.defl.c_str(), doc.GetAllocator()), doc.GetAllocator());
        paramArray.PushBack(Value().SetString(param.doc.c_str(), doc.GetAllocator()), doc.GetAllocator());
        params.PushBack(paramArray, doc.GetAllocator());
    }

    // Categories array
    Value categories(kArrayType);
    for (const auto& category : descriptor.categories) {
        categories.PushBack(Value().SetString(category.c_str(), doc.GetAllocator()), doc.GetAllocator());
    }

    // Push values into the main document
    doc.PushBack(Value().SetString(key.c_str(), doc.GetAllocator()), doc.GetAllocator());
    doc.PushBack(inputs, doc.GetAllocator());
    doc.PushBack(outputs, doc.GetAllocator());
    doc.PushBack(params, doc.GetAllocator());
    doc.PushBack(categories, doc.GetAllocator());
    doc.PushBack(Value().SetString(descriptor.doc.c_str(), doc.GetAllocator()), doc.GetAllocator());

    // Write the JSON string to stdout
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    doc.Accept(writer);
    return buffer.GetString();
}
}


ZENO_API UserData &Session::userData() const {
    return *m_userData;
}



ZENO_API Session &getSession() {
    if (!s_ptrGlobal) {
        throw makeError<UnimplError>("global application has not been initialized");
    }
    return *s_ptrGlobal;
}

std::unique_ptr<Session> createApplication() {
    std::unique_ptr<Session> ptr = std::make_unique<Session>();
    s_ptrGlobal = ptr.get();
    return ptr;
}

}

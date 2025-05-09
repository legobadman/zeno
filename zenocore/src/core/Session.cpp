#include <zeno/core/Session.h>
#include <zeno/core/IObject.h>
#include <zeno/core/INodeClass.h>
#include <zeno/core/Assets.h>
#include <zeno/core/ObjectManager.h>
#include <zeno/extra/GlobalState.h>
#include <zeno/extra/GlobalComm.h>
#include <zeno/extra/GlobalError.h>
#include <zeno/extra/EventCallbacks.h>
#include <zeno/types/UserData.h>
#include <zeno/core/Graph.h>
#include <zeno/core/NodeImpl.h>
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


#include <reflect/core.hpp>
#include <reflect/type.hpp>
#include <reflect/metadata.hpp>
#include <reflect/registry.hpp>
#include <reflect/container/any>
#include <reflect/container/arraylist>
#include <zeno/core/reflectdef.h>
#include "zeno_types/reflect/reflection.generated.hpp"
//#include "zeno_nodes/reflect/reflection.generated.hpp"

//#include <Python.h>
//#include <pybind11/pybind11.h>


using namespace zeno::reflect;
using namespace zeno::types;

namespace zeno {

    struct _ObjUIInfo
    {
        std::string_view name;
        std::string_view color;
    };
    static std::map<size_t, _ObjUIInfo> s_objsUIInfo;

ZENO_API Session::Session()
    : globalState(std::make_unique<GlobalState>())
    , globalComm(std::make_unique<GlobalComm>())
    , globalError(std::make_unique<GlobalError>())
    , eventCallbacks(std::make_unique<EventCallbacks>())
    , m_userData(std::make_unique<UserData>())
    , mainGraph(std::make_shared<Graph>("main"))
    , assets(std::make_shared<AssetsMgr>())
    , objsMan(std::make_unique<ObjectManager>())
    , globalVariableManager(std::make_unique<GlobalVariableManager>())
    , funcManager(std::make_unique<FunctionManager>())
{
}

ZENO_API Session::~Session() = default;


static CustomUI descToCustomui(const Descriptor& desc) {
    //兼容以前写的各种ZENDEFINE
    CustomUI ui;

    ui.nickname = desc.displayName;
    ui.uistyle.iconResPath = desc.iconResPath;
    ui.doc = desc.doc;
    if (!desc.categories.empty())
        ui.category = desc.categories[0];   //很多cate都只有一个

    ParamGroup default;
    for (const SocketDescriptor& param_desc : desc.inputs) {
        if (param_desc._desc.type == Desc_Prim) {
            //有可能没赋初值，要检查一下
            if (!param_desc._desc.primparam.defl.has_value()) {
                ParamPrimitive newparam = param_desc._desc.primparam;
                newparam.defl = initAnyDeflValue(newparam.type);
                default.params.push_back(newparam);
            }
            else {
                //字符串的defl都是以const char*作为内部储存的类型，这里转一下string，以免上层逻辑处理疏漏
                ParamPrimitive newparam = param_desc._desc.primparam;
                if (newparam.defl.type() == zeno::reflect::type_info<const char*>()) {
                    std::string str = zeno::reflect::any_cast<const char*>(newparam.defl);
                    newparam.defl = str;
                }
                default.params.push_back(newparam);
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
                default.params.push_back(param);
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
                if (type == gParamType_Dict || type == gParamType_List) {
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
        default.params.push_back(param);
    }
    for (const SocketDescriptor& param_desc : desc.outputs) {
        if (param_desc._desc.type == Desc_Prim) {
            ui.outputObjs.push_back(param_desc._desc.objparam);
        }
        else if (param_desc._desc.type == Desc_Obj) {
            ui.outputPrims.push_back(param_desc._desc.primparam);
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
    tab.groups.emplace_back(std::move(default));
    ui.inputPrims.emplace_back(std::move(tab));
    return ui;
}

ZENO_API void Session::defNodeClass(INode* (*ctor)(), std::string const &clsname, Descriptor const &desc) {
    if (clsname == "Subnet") {
        int j;
        j = 0;
    }
    
    if (nodeClasses.find(clsname) != nodeClasses.end()) {
        log_warn("node class redefined: `{}`\n", clsname);
        return;
    }

    CustomUI ui = descToCustomui(desc);
    auto cls = std::make_unique<ImplNodeClass>(ctor, ui, clsname);
    if (!clsname.empty() && clsname.front() == '^')
        return;

    NodeInfo info;
    info.module_path = m_current_loading_module;
    info.name = clsname;
    info.status = ZModule_Loaded;
    info.cate = cls->m_customui.category;

    m_cates.push_back(std::move(info));
    nodeClasses.emplace(clsname, std::move(cls));
}

ZENO_API void Session::defNodeClass2(INode* (*ctor)(), std::string const& nodecls, CustomUI const& customui) {
    if (nodeClasses.find(nodecls) != nodeClasses.end()) {
        log_error("node class redefined: `{}`\n", nodecls);
    }
    CustomUI ui = customui;
    initControlsByType(ui);
    auto cls = std::make_unique<ImplNodeClass>(ctor, ui, nodecls);
    nodeClasses.emplace(nodecls, std::move(cls));
}

ZENO_API void Session::defNodeClass3(INode* (*ctor)(), const char* pName, Descriptor const& desc) {
    //auto cls = std::make_unique<ImplNodeClass>(ctor, desc, pName);
    //nodeClasses.emplace(pName, std::move(cls));
}

#if 0
ZENO_API void Session::defNodeReflectClass(std::function<INode*()> ctor, zeno::reflect::TypeBase* pTypeBase)
{
    assert(pTypeBase);
    const zeno::reflect::ReflectedTypeInfo& info = pTypeBase->get_info();
    auto& nodecls = std::string(info.qualified_name.c_str());
    //有些name反射出来可能带有命名空间比如zeno::XXX
    int idx = nodecls.find_last_of(':');
    if (idx != std::string::npos) {
        nodecls = nodecls.substr(idx + 1);
    }

    if (nodeClasses.find(nodecls) != nodeClasses.end()) {
        //log_error("node class redefined: `{}`\n", nodecls);
        return;
    }
    auto cls = std::make_unique<ReflectNodeClass>(ctor, nodecls, pTypeBase);
    std::string cate = cls->m_customui.category;
    if (m_cates.find(cate) == m_cates.end())
        m_cates.insert(std::make_pair(cate, std::vector<std::string>()));
    m_cates[cate].push_back(nodecls);

    nodeClasses.emplace(nodecls, std::move(cls));
}
#endif

void Session::beginLoadModule(const std::string& module_name) {
    m_current_loading_module = module_name;
}

void Session::uninstallModule(const std::string& module_path) {
    for (NodeInfo& info : m_cates) {
        if (info.module_path == module_path) {
            info.status = ZModule_UnLoaded;
            const std::string uninstall_nodecls = info.name;
            nodeClasses.erase(uninstall_nodecls);
            //所有节点要disable掉，只留一个空壳
            mainGraph->update_load_info(uninstall_nodecls, true);
        }
    }
}

void Session::endLoadModule() {
    //有可能加载模块前有旧节点，这时候要enable已有的节点
    for (NodeInfo& info : m_cates) {
        if (info.module_path == m_current_loading_module) {
            info.status = ZModule_Loaded;
            mainGraph->update_load_info(info.name, false);
        }
    }
    m_current_loading_module = "";
}


ZENO_API INodeClass::INodeClass(CustomUI const &customui, std::string const& classname)
    : m_customui(customui)
    , classname(classname)
{
}

ZENO_API INodeClass::~INodeClass() = default;

ZENO_API std::shared_ptr<Graph> Session::createGraph(const std::string& name) {
    auto graph = std::make_shared<Graph>(name);
    return graph;
}

ZENO_API NodeImpl* Session::getNodeByUuidPath(std::string const& uuid_path) {
    return mainGraph->getNodeByUuidPath(uuid_path);
}

ZENO_API void Session::resetMainGraph() {
    mainGraph.reset();
    mainGraph = std::make_shared<Graph>("main");
    globalVariableManager.reset();
    globalVariableManager = std::make_unique<GlobalVariableManager>();
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
                run();
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

ZENO_API void Session::registerCommitRender(F_CommitRender&& func) {
    m_func_commitrender = func;
}

ZENO_API std::shared_ptr<Graph> Session::getGraphByPath(const std::string& path) {
    //对于assets
    //可能的形式包括： /ABC/subnet1/subnet2   ABC   /ABC
    std::vector<std::string> items = split_str(path, '/', false);
    if (items.empty()) {
        return nullptr;
    }

    std::string graph_name = items[0];
    if (graph_name == "main") {
        return mainGraph->getGraphByPath(path);
    }
    else {
        if (auto spGraph = assets->getAssetGraph(graph_name, true)) {
            return spGraph->getGraphByPath(path);
        }
        return nullptr;
    }
}

void Session::commit_to_render(render_update_info info) {
    if (m_func_commitrender) {
        m_func_commitrender(info);
    }
}

void Session::reportNodeStatus(const ObjPath& path, bool bDirty, NodeRunStatus status)
{
    if (m_funcNodeStatus) {
        m_funcNodeStatus(path, bDirty, status);
    }
}

ZENO_API int Session::registerObjId(const std::string& objprefix)
{
    int objid = objsMan->registerObjId(objprefix);
    return objid;
}

ZENO_API void Session::switchToFrame(int frameid)
{
    CORE_API_BATCH
    mainGraph->markDirtyWhenFrameChanged();
    globalState->updateFrameId(frameid);
}

ZENO_API void Session::updateFrameRange(int start, int end)
{
    CORE_API_BATCH
    globalState->updateFrameRange(start, end);
}

ZENO_API void Session::interrupt() {
    m_bInterrupted = true;
}

ZENO_API bool Session::is_interrupted() const {
    return m_bInterrupted;
}

ZENO_API bool Session::run(const std::string& currgraph) {
    if (m_bDisableRunning)
        return false;

    if (m_bReentrance) {
        return true;
    }

    m_bReentrance = true;
    m_bInterrupted = false;
    globalState->set_working(true);

    objsMan->beforeRun();
    zeno::scope_exit sp([&]() { 
        objsMan->afterRun();
        m_bReentrance = false;
    });

    globalError->clearState();

    //本次运行清除m_objects中上一次运行时被标记移除的obj，不能立刻清除因为视窗export_loading_objs时，需要在m_objects中查找被删除的obj
    objsMan->clearLastUnregisterObjs();
    //对之前删除节点时记录的obj，对应的所有其他关联节点，都标脏
    objsMan->remove_attach_node_by_removing_objs();

    if (!currgraph.empty()) {
        getGraphByPath(currgraph);
    }
    else {
        mainGraph->runGraph();
    }
    return true;
}

ZENO_API void Session::set_auto_run(bool bOn) {
    m_bAutoRun = bOn;
}

ZENO_API bool Session::is_auto_run() const {
    return m_bAutoRun;
}

ZENO_API void Session::set_Rerun()
{
    mainGraph->markDirtyAll();
}

ZENO_API void Session::registerObjUIInfo(size_t hashcode, std::string_view color, std::string_view nametip) {
    s_objsUIInfo.insert(std::make_pair(hashcode, _ObjUIInfo { nametip, color }));
}

ZENO_API bool Session::getObjUIInfo(size_t hashcode, std::string_view& color, std::string_view& nametip) {
    auto iter = s_objsUIInfo.find(hashcode);
    if (iter == s_objsUIInfo.end()) {
        color = "#000000";
        nametip = "unknown type";
        return false;
    }
    color = iter->second.color;
    nametip = iter->second.name;
    return true;
}

ZENO_API void Session::initEnv(const zenoio::ZSG_PARSE_RESULT ioresult) {

    bool bDisableRun = m_bDisableRunning;
    m_bDisableRunning = true;
    scope_exit sp([&]() {m_bDisableRunning = bDisableRun; });

    resetMainGraph();
    mainGraph->init(ioresult.mainGraph);
    mainGraph->initRef(ioresult.mainGraph);
    //referManager->init(mainGraph);

    switchToFrame(ioresult.timeline.currFrame);
    //init $F globalVariable
    //zeno::getSession().globalVariableManager->overrideVariable(zeno::GVariable("$F", zeno::reflect::make_any<float>(ioresult.timeline.currFrame)));
}

ZENO_API zeno::NodeRegistry Session::dumpCoreCates() {
    return m_cates;
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

ZENO_API std::string Session::dumpDescriptorsJSON() const {
    //deprecated.
    return "";
}

ZENO_API UserData &Session::userData() const {
    return *m_userData;
}

ZENO_API Session &getSession() {
#if 0
    static std::unique_ptr<Session> ptr;
    if (!ptr) {
        ptr = std::make_unique<Session>();
    }
#else
    static std::unique_ptr<Session> ptr = std::make_unique<Session>();
#endif
    return *ptr;
}

//namespace py = pybind11;
//
//PYBIND11_MODULE(ze, z) {
//    py::class_<Session>(z, "Session")
//        .def("run", &Session::run)
//        .def("interrupt", &Session::interrupt);
//}

}

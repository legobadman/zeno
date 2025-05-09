#include <zeno/core/NodeImpl.h>
#include <zeno/core/Graph.h>
#include <zeno/core/Descriptor.h>
#include <zeno/core/Session.h>
#include <zeno/core/Assets.h>
#include <zeno/core/ObjectManager.h>
#include <zeno/core/INodeClass.h>
#include <zeno/types/DummyObject.h>
#include <zeno/types/NumericObject.h>
#include <zeno/types/StringObject.h>
#include <zeno/extra/GlobalState.h>
#include <zeno/extra/DirtyChecker.h>
#include <zeno/extra/TempNode.h>
#include <zeno/utils/Error.h>
#include <zeno/utils/string.h>
#include <zeno/funcs/ParseObjectFromUi.h>
#ifdef ZENO_BENCHMARKING
#include <zeno/utils/Timer.h>
#endif
#include <zeno/core/IObject.h>
#include <zeno/utils/safe_at.h>
#include <zeno/utils/logger.h>
#include <zeno/utils/uuid.h>
#include <zeno/extra/GlobalState.h>
#include <zeno/core/CoreParam.h>
#include <zeno/types/DictObject.h>
#include <zeno/ListObject.h>
#include <zeno/utils/helper.h>
#include <zeno/utils/uuid.h>
#include <zeno/extra/SubnetNode.h>
#include <zeno/extra/GraphException.h>
#include <zeno/formula/formula.h>
#include <zeno/core/FunctionManager.h>
#include <reflect/type.hpp>
#include <reflect/container/arraylist>
#include <reflect/metadata.hpp>
#include <zeno/types/ListObject_impl.h>
#include <zeno/types/MeshObject.h>
#include <zeno/types/MatrixObject.h>
#include "zeno_types/reflect/reflection.generated.hpp"
#include <zeno/core/reflectdef.h>
#include <zeno/formula/zfxexecute.h>
#include <zeno/extra/CalcContext.h>
#include <zeno/extra/SubnetNode.h>
#include <zeno/utils/interfaceutil.h>
//#include <Python.h>
//#include <pybind11/pybind11.h>


using namespace zeno::reflect;
using namespace zeno::types;


namespace zeno {

    class ForEachEnd;

NodeImpl::NodeImpl(INode* pNode) : m_pNode(pNode), m_pGraph(nullptr) {
    if (m_pNode)
        m_pNode->m_pAdapter = this;
}

void NodeImpl::initUuid(Graph* pGraph, const std::string nodecls) {
    //TODO: 考虑asset的情况
    m_nodecls = nodecls;
    this->m_pGraph = pGraph;

    m_uuid = generateUUID(nodecls);
    ObjPath path;
    path += m_uuid;
    while (pGraph) {
        const std::string name = pGraph->getName();
        if (name == "main") {
            break;
        }
        else {
            if (!pGraph->getParentSubnetNode())
                break;
            auto pSubnetNode = pGraph->getParentSubnetNode();
            assert(pSubnetNode);
            path = (pSubnetNode->m_uuid) + "/" + path;
            pGraph = pSubnetNode->m_pGraph;
        }
    }
    m_uuidPath = path;
}

NodeImpl::~NodeImpl() {
    int j;
    j = 0;
}

Graph* NodeImpl::getThisGraph() const {
    return m_pGraph;
}

Session *NodeImpl::getThisSession() const {
    return &getSession();
}

GlobalState *NodeImpl::getGlobalState() const {
    return getSession().globalState.get();
}

std::string NodeImpl::get_nodecls() const
{
    return m_nodecls;
}

std::string NodeImpl::get_ident() const
{
    return m_name;
}

std::string NodeImpl::get_show_name() const {
    if (nodeClass) {
        std::string dispName = nodeClass->m_customui.nickname;
        if (!dispName.empty())
            return dispName;
    }
    return m_nodecls;
}

std::string NodeImpl::get_show_icon() const {
    if (nodeClass) {
        return nodeClass->m_customui.uistyle.iconResPath;
    }
    else {
        return "";
    }
}

ObjPath NodeImpl::get_uuid_path() const {
    return m_uuidPath;
}

CustomUI NodeImpl::get_customui() const {
    if (nodeClass) {
        return nodeClass->m_customui;
    }
    else {
        return CustomUI();
    }
}

Graph* NodeImpl::getGraph() const {
    return m_pGraph;
}

INode* NodeImpl::coreNode() const {
    return m_pNode.get();
}

ObjPath NodeImpl::get_graph_path() const {
    ObjPath path;
    path = "";

    Graph* pGraph = m_pGraph;

    while (pGraph) {
        const std::string name = pGraph->getName();
        if (name == "main") {
            path = "/main/" + path;
            break;
        }
        else {
            if (!pGraph->getParentSubnetNode())
                break;
            auto pSubnetNode = pGraph->getParentSubnetNode();
            assert(pSubnetNode);
            path = pSubnetNode->m_name + "/" + path;
            pGraph = pSubnetNode->m_pGraph;
        }
    }
    return path;
}

CustomUI NodeImpl::_deflCustomUI() const {
    if (!nodeClass)
        return CustomUI();

    std::set<std::string> intputPrims, outputPrims, inputObjs, outputObjs;
    zeno::CustomUI origin = nodeClass->m_customui;
    zeno::CustomUI exportui = origin;

    for (auto& input_param : exportui.inputObjs) {
        std::string name = input_param.name;
        auto iterObj = m_inputObjs.find(name);
        assert(iterObj != m_inputObjs.end());
        input_param = iterObj->second.exportParam();
    }

    for (auto& tab : exportui.inputPrims) {
        for (auto& group : tab.groups) {
            for (auto& input_param : group.params) {
                std::string name = input_param.name;
                auto iterPrim = m_inputPrims.find(name);
                assert(iterPrim != m_inputPrims.end());
                input_param = iterPrim->second.exportParam();
            }
        }
    }

    for (auto& output_param : exportui.outputObjs) {
        std::string name = output_param.name;
        auto iterObj = m_outputObjs.find(name);
        assert(iterObj != m_outputObjs.end());
        output_param = iterObj->second.exportParam();
    }

    for (auto& output_param : exportui.outputPrims) {
        std::string name = output_param.name;
        auto iterPrim = m_outputPrims.find(name);
        assert(iterPrim != m_outputPrims.end());
        output_param = iterPrim->second.exportParam();
    }
    return exportui;
}

CustomUI NodeImpl::export_customui() const
{
    CustomUI deflUI = _deflCustomUI();
    INode* pNode = coreNode();
    if (pNode) {
        CustomUI nodeui = pNode->export_customui();
        if (nodeui.inputObjs.empty() && nodeui.inputPrims.empty() &&
            nodeui.outputPrims.empty() && nodeui.outputObjs.empty()) {
            if (nodeui.uistyle.background != "" || nodeui.uistyle.iconResPath != "") {
                deflUI.uistyle = nodeui.uistyle;
            }
            return deflUI;
        }
        else {
            return nodeui;
        }
    }
    return deflUI;

#if 0
    exportui.nickname = origin.nickname;
    exportui.uistyle = origin.uistyle;
    exportui.doc = origin.doc;
    if (!origin.category.empty())
        exportui.category = origin.category;

    zeno::ParamGroup exportgroup;
    zeno::ParamTab exporttab;
    if (!origin.inputPrims.empty()) {
        exporttab.name = origin.inputPrims[0].name;
        if (!origin.inputPrims[0].groups.empty()) {
            exportgroup.name = origin.inputPrims[0].groups[0].name;
        }
    }
    for (const zeno::ParamTab& tab : origin.inputPrims) {
        for (const zeno::ParamGroup& group : tab.groups) {
            for (const zeno::ParamPrimitive& param : group.params) {
                auto iter = m_inputPrims.find(param.name);
                if (iter != m_inputPrims.end()) {
                    exportgroup.params.push_back(iter->second.exportParam());
                    intputPrims.insert(param.name);
                }
            }
        }
    }
    for (const zeno::ParamPrimitive& param : origin.outputPrims) {
        auto iter = m_outputPrims.find(param.name);
        if (iter != m_outputPrims.end()) {
            exportui.outputPrims.push_back(iter->second.exportParam());
            outputPrims.insert(param.name);
        }
    }
    for (const auto& param : origin.inputObjs) {
        auto iter = m_inputObjs.find(param.name);
        if (iter != m_inputObjs.end()) {
            exportui.inputObjs.push_back(iter->second.exportParam());
            inputObjs.insert(param.name);
        }
    }
    for (const auto& param : origin.outputObjs) {
        auto iter = m_outputObjs.find(param.name);
        if (iter != m_outputObjs.end()) {
            exportui.outputObjs.push_back(iter->second.exportParam());
            outputObjs.insert(param.name);
        }
    }
    exporttab.groups.emplace_back(std::move(exportgroup));
    exportui.inputPrims.emplace_back(std::move(exporttab));
    for (auto& [key, param] : m_inputPrims) {
        if (intputPrims.find(key) == intputPrims.end())
            exportui.inputPrims[0].groups[0].params.push_back(param.exportParam());
    }
    for (auto& [key, param] : m_outputPrims) {
        if (outputPrims.find(key) == outputPrims.end())
            exportui.outputPrims.push_back(param.exportParam());
    }
    for (auto& [key, param] : m_inputObjs) {
        if (inputObjs.find(key) == inputObjs.end())
            exportui.inputObjs.push_back(param.exportParam());
    }
    for (auto& [key, param] : m_outputObjs) {
        if (outputObjs.find(key) == outputObjs.end())
            exportui.outputObjs.push_back(param.exportParam());
    }
    return exportui;
#endif
}

ObjPath NodeImpl::get_path() const {
    ObjPath path;
    path = m_name;

    Graph* pGraph = m_pGraph;

    while (pGraph) {
        const std::string name = pGraph->getName();
        if (name == "main") {
            path = "/main/" + path;
            break;
        }
        else {
            path = name + "/" + path;
            if (!pGraph->getParentSubnetNode())
                break;
            auto pSubnetNode = pGraph->getParentSubnetNode();
            assert(pSubnetNode);
            pGraph = pSubnetNode->m_pGraph;
        }
    }
    return path;
}

std::string NodeImpl::get_uuid() const
{
    return m_uuid;
}

std::string NodeImpl::get_name() const
{
    return m_name;
}

void NodeImpl::set_name(const std::string& customname)
{
    m_name = customname;
}

void NodeImpl::set_view(bool bOn)
{
    //这一类方法要和ui model端位于同一线程
    if (m_bView == bOn) {
        return;
    }
    {
        CORE_API_BATCH

        m_bView = bOn;
        CALLBACK_NOTIFY(set_view, m_bView)

        Graph* spGraph = m_pGraph;
        assert(spGraph);
        spGraph->viewNodeUpdated(m_name, bOn);
    }
}

bool NodeImpl::is_view() const
{
    return m_bView;
}

void NodeImpl::set_mute(bool bOn)
{
    CORE_API_BATCH

    m_mute = bOn;
    CALLBACK_NOTIFY(set_mute, m_mute)
    mark_dirty(true);

    //Graph* spGraph = graph;
    //assert(spGraph);
    //spGraph->viewNodeUpdated(m_name, bOn);
}

bool NodeImpl::is_mute() const
{
    return m_mute;
}

void NodeImpl::reportStatus(bool bDirty, NodeRunStatus status) {
    m_status = status;
    m_dirty = bDirty;
    zeno::getSession().reportNodeStatus(m_uuidPath, bDirty, status);
}

void NodeImpl::mark_previous_ref_dirty() {
    mark_dirty(true);
    //不仅要自身标脏，如果前面的节点是以引用的方式连接，说明前面的节点都可能被污染了，所有都要标脏。
    //TODO: 由端口而不是边控制。
    /*
    for (const auto& [name, param] : m_inputs) {
        for (const auto& link : param.links) {
            if (link->lnkProp == Link_Ref) {
                auto spOutParam = link->fromparam.lock();
                auto spPreviusNode = spOutParam->m_wpNode;
                spPreviusNode->mark_previous_ref_dirty();
            }
        }
    }
    */
}

bool NodeImpl::has_frame_relative_params() const {
    for (auto& [name, param] : m_inputPrims) {
        assert(param.defl.has_value());
        const std::string& uuid = get_uuid();
        if (gParamType_String == param.type) {
            if (param.defl.type().hash_code() == gParamType_PrimVariant) {//type是string，实际defl可能是primvar
                const zeno::PrimVar& editVar = zeno::reflect::any_cast<zeno::PrimVar>(param.defl);
                return std::visit([](auto&& arg)-> bool {
                    using T = std::decay_t<decltype(arg)>;
                    if constexpr (std::is_same_v<T, std::string>) {
                        std::string defl = zeno::reflect::any_cast<std::string>(arg);
                        if (defl.find("$F") != std::string::npos)
                            return true;
                    }
                    return false;
                }, editVar);
            } else if (param.defl.type().hash_code() == gParamType_String) {
                std::string defl = zeno::any_cast_to_string(param.defl);
                if (defl.find("$F") != std::string::npos) {
                    return true;
                }
            }
        }
        else if (gParamType_Int == param.type || gParamType_Float == param.type || gParamType_PrimVariant == param.type) {
            assert(gParamType_PrimVariant == param.defl.type().hash_code());
            const zeno::PrimVar& editVar = zeno::reflect::any_cast<zeno::PrimVar>(param.defl);
            bool bFind = std::visit([=](auto&& arg)->bool {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, std::string>) {
                    if (arg.find("$F") != std::string::npos) {
                        return true;
                    }
                }
                return false;
            }, editVar);
            if (bFind)
                return true;
        }
        else if (gParamType_Vec2f == param.type ||
            gParamType_Vec2i == param.type ||
            gParamType_Vec3f == param.type ||
            gParamType_Vec3i == param.type ||
            gParamType_Vec4f == param.type ||
            gParamType_Vec4i == param.type)
        {
            assert(gParamType_VecEdit == param.defl.type().hash_code());
            const zeno::vecvar& editVar = zeno::reflect::any_cast<zeno::vecvar>(param.defl);
            for (auto primvar : editVar) {
                bool bFind = std::visit([=](auto&& arg)->bool {
                    using T = std::decay_t<decltype(arg)>;
                    if constexpr (std::is_same_v<T, std::string>) {
                        if (arg.find("$F") != std::string::npos) {
                            return true;
                        }
                    }
                    return false;
                }, primvar);
                if (bFind)
                    return true;
            }
        }
    }
    return false;
}

bool NodeImpl::isInDopnetwork()
{
    Graph* parentGraph = m_pGraph;
    while (parentGraph)
    {
        if (auto pSubnetImpl = parentGraph->getParentSubnetNode())
        {
            if (SubnetNode* subnet = getSubnetNode(pSubnetImpl)) {
                if (DopNetwork* dop = dynamic_cast<DopNetwork*>(subnet)) {
                    return true;
                }
                else {
                    parentGraph = pSubnetImpl->getGraph();
                }
            }
            else {
                break;
            }
        }
        else break;
    }
    return false;
}

void NodeImpl::onInterrupted() {
    mark_dirty(true);
    mark_previous_ref_dirty();
}

void NodeImpl::mark_dirty(bool bOn, DirtyReason reason, bool bWholeSubnet, bool bRecursively)
{
    scope_exit sp([&] {
        m_status = Node_DirtyReadyToRun;  //修改了数据，标脏，并置为此状态。（后续在计算过程中不允许修改数据，所以markDirty理论上是前端驱动）
        reportStatus(m_dirty, m_status);
    });

    if (m_dirty == bOn)
        return;

    m_dirty = bOn;

    if (!bRecursively)
        return;

    if (m_dirty) {
        for (auto& [name, param] : m_inputPrims) {
            for (auto link : param.reflinks) {
                if (link->dest_inparam != &param) {
                    assert(link->dest_inparam);
                    auto destNode = link->dest_inparam->m_wpNode;
                    destNode->mark_dirty(true);
                }
            }
        }
        for (auto& [name, param] : m_outputObjs) {
            for (auto link : param.reflinks) {
                assert(link->dest_inparam);
                auto destNode = link->dest_inparam->m_wpNode;
                destNode->mark_dirty(true);
            }
            for (auto link : param.links) {
                auto inParam = link->toparam;
                assert(inParam);
                if (inParam) {
                    auto inNode = inParam->m_wpNode;
                    assert(inNode);
                    inNode->mark_dirty(true);
                }
            }
        }
        for (auto& [name, param] : m_outputPrims) {
            for (auto link : param.links) {
                auto inParam = link->toparam;
                assert(inParam);
                if (inParam) {
                    auto inNode = inParam->m_wpNode;
                    assert(inNode);
                    inNode->mark_dirty(true);
                }
            }
        }
    }

    if (SubnetNode* pSubnetNode = dynamic_cast<SubnetNode*>(this))
    {
        if (bWholeSubnet)
            pSubnetNode->mark_subnetdirty(bOn);
        if (DopNetwork* pDop = dynamic_cast<DopNetwork*>(pSubnetNode)) {
            pDop->resetFrameState();
    }
    }

    Graph* spGraph = m_pGraph;
    assert(spGraph);
    if (auto pSubnetImpl = spGraph->getParentSubnetNode())
    {
        pSubnetImpl->mark_dirty(true, Dirty_All, false);
    }
}

void NodeImpl::mark_dirty_objs()
{
    for (auto const& [name, param] : m_outputObjs)
    {
        if (param.spObject) {
            assert(param.spObject);
            if (param.spObject->key().empty()) {
                continue;
            }
            getSession().objsMan->collect_removing_objs(zsString2Std(param.spObject->key()));
        }
    }
}

void NodeImpl::complete() {}

void NodeImpl::preApply(CalcContext* pContext) {
    if (!m_dirty)
        return;

    //debug
#if 0
    if (m_name == "Seed") {
        int j;
        j = 0;
    }
#endif

    reportStatus(true, Node_Pending);

    //TODO: the param order should be arranged by the descriptors.
    for (const auto& [name, param] : m_inputObjs) {
        bool ret = requireInput(name, pContext);
        if (!ret)
            zeno::log_warn("the param {} may not be initialized", name);
    }
    for (const auto& [name, param] : m_inputPrims) {
        bool ret = requireInput(name, pContext);
        if (!ret)
            zeno::log_warn("the param {} may not be initialized", name);
    }
}

void NodeImpl::preApply_Primitives(CalcContext* pContext) {
    if (!m_dirty)
        return;
    for (const auto& [name, param] : m_inputPrims) {
        bool ret = requireInput(name, pContext);
        if (!ret)
            zeno::log_warn("the param {} may not be initialized", name);
    }
}

void NodeImpl::preApplyTimeshift(CalcContext* pContext)
{
    int oldFrame = getSession().globalState->getFrameId();
    scope_exit sp([&oldFrame] { getSession().globalState->updateFrameId(oldFrame); });
    //get offset
    auto defl = get_input_prim_param("offset").defl;
    zeno::PrimVar offset = defl.has_value() ? zeno::reflect::any_cast<zeno::PrimVar>(defl) : 0;
    int newFrame = oldFrame + std::get<int>(offset);
    //clamp
    auto startFrameDefl = get_input_prim_param("start frame").defl;
    int globalStartFrame = getSession().globalState->getStartFrame();
    int startFrame = startFrameDefl.has_value() ? std::get<int>(zeno::reflect::any_cast<PrimVar>(startFrameDefl)) : globalStartFrame;
    auto endFrameDefl = get_input_prim_param("end frame").defl;
    int globalEndFrame = getSession().globalState->getEndFrame();
    int endFrame = endFrameDefl.has_value() ? std::get<int>(zeno::reflect::any_cast<PrimVar>(endFrameDefl)) : globalEndFrame;
    auto clampDefl = get_input_prim_param("clamp").defl;
    std::string clamp = clampDefl.has_value() ? zeno::reflect::any_cast<std::string>(clampDefl) : "None";
    if (startFrame > endFrame) {
        startFrame = globalStartFrame;
        endFrame = globalEndFrame;
    }
    if (clamp == "Clamp to First") {
        newFrame = newFrame < startFrame ? startFrame : newFrame;
    }
    else if (clamp == "Clamp to Last") {
        newFrame = newFrame > endFrame ? endFrame : newFrame;
    }
    else if (clamp == "Clamp to Both") {
        if (newFrame < startFrame) {
            newFrame = startFrame;
        }
        else if (newFrame > endFrame) {
            newFrame = endFrame;
        }
    }
    getSession().globalState->updateFrameId(newFrame);
    //propaget dirty
    propagateDirty(this, "$F");

    preApply(pContext);
}

void NodeImpl::reflectForeach_apply(CalcContext* pContext)
{
    std::string foreach_begin_path = zeno::any_cast_to_string(get_defl_value("ForEachBegin Path"));
    if (Graph* spGraph = m_pGraph) {
        auto foreach_begin = spGraph->getNode(foreach_begin_path);

        auto foreach_end = coreNode();
        assert(foreach_end);
        for (foreach_end->reset_forloop_settings(); foreach_end->is_continue_to_run(); foreach_end->increment())
        {
            foreach_begin->mark_dirty(true);
            pContext->curr_iter = zeno::reflect::any_cast<int>(foreach_begin->get_defl_value("Current Iteration"));

            preApply(pContext);
            apply();
        }
        auto output = get_output_obj("Output Object");
        if (output)
            output->update_key(stdString2zs(m_uuid));
    }
}


void NodeImpl::apply() {
    if (m_pNode) {
        m_pNode->apply();
    }
    else {
        throw makeError<UnimplError>("the node has been uninstalled");
    }
}

void NodeImpl::reflectNode_apply()
{
    assert(false);
#if 0
    if (m_pTypebase) {
        for (zeno::reflect::IMemberFunction* func : m_pTypebase->get_member_functions()) {
            const auto& funcname = func->get_name();
            if (funcname == "apply") {
                //根据ReflectCustomUI获取fieldName到displayName映射
                std::map<std::string, std::string> mappingInputParams;
                std::vector<std::string> mappingReturnParams;

                zeno::_ObjectParam retInfoOnReflectUI;
                getNameMappingFromReflectUI(m_pTypebase, this, mappingInputParams, mappingReturnParams);

                //从apply参数获取输入
                zeno::reflect::ArrayList<zeno::reflect::Any> paramValues;
                std::vector<std::tuple<std::string, zeno::ParamType, int>> outputsName;

                const zeno::reflect::ArrayList<zeno::reflect::RTTITypeInfo>& params = func->get_params();
                const auto& field_names = func->get_params_name();
                for (int i = 0; i < params.size(); i++) {
                    const zeno::reflect::RTTITypeInfo& param_type = params[i];
                    const std::string field_name(field_names[i].c_str());
                    std::string normal_name = field_name;
                    auto iterMapping = mappingInputParams.find(field_name);
                    if (iterMapping != mappingInputParams.end()) {
                        normal_name = iterMapping->second;
                    }

                    zeno::reflect::Any inputAny;
                    bool bConstPtr = false;
                    zeno::isObjectType(param_type, bConstPtr);

                    auto iter = m_inputPrims.find(normal_name);
                    if (iter != m_inputPrims.end()) {
                        auto& val = iter->second.result;
                        if (val.has_value()) {
                            inputAny = val;
                        }
                        else {
                            inputAny = iter->second.defl;
                        }
                    }
                    else {
                        auto iter2 = m_inputObjs.find(normal_name);
                        if (iter2 != m_inputObjs.end()) {
                            inputAny = iter2->second.spObject;
                        }
                    }
                    //类型不一致，数值类型可能需要转换
                    ParamType outtype = inputAny.type().hash_code();
                    ParamType intype = param_type.hash_code();
                    if (intype != outtype && isNumericVecType(outtype)) {
                        inputAny = convertNumericAnyType(outtype, intype, inputAny);
                    }
                    paramValues.add_item(inputAny);
                }

                //从输入到成员变量
                for (zeno::reflect::IMemberField* field : m_pTypebase->get_member_fields()) {
                    std::string field_name(field->get_name().c_str());
                    std::string param_name;
                    if (const zeno::reflect::IRawMetadata* metadata = field->get_metadata()) {
                    if (const zeno::reflect::IMetadataValue* value = metadata->get_value("Role")) {
                            int _role = value->as_int();
                            if (_role == Role_InputPrimitive || _role == Role_InputObject) {
                                if (const zeno::reflect::IMetadataValue* value = metadata->get_value("DisplayName"))
                                    param_name = value->as_string();
                                else {
                                    param_name = field_name;
                                }
                                zeno::reflect::Any inputAny;
                                auto iter = m_inputPrims.find(param_name);
                                if (iter != m_inputPrims.end()) {
                                    auto& val = iter->second.result;
                                    if (val.has_value()) {
                                        inputAny = val;
                                    }
                                    else {
                                        inputAny = iter->second.defl;
                                    }
                                }
                                else {
                                    auto iter2 = m_inputObjs.find(param_name);
                                    if (iter2 != m_inputObjs.end()) {
                                        inputAny = iter2->second.spObject;
                                }
                                }
                                //类型不一致，数值类型可能需要转换
                                ParamType outtype = inputAny.type().hash_code();
                                ParamType intype = field->get_field_type().type_hash();
                                if (intype != outtype && isNumericVecType(outtype)) {
                                    inputAny = convertNumericAnyType(outtype, intype, inputAny);
                                }
                                if (inputAny.has_value())
                                    field->set_field_value(this, inputAny);
                            }
                        }
                    }
                }

                //调用apply
                zeno::reflect::Any res = func->invoke_unsafe(this, paramValues);

                const zeno::reflect::RTTITypeInfo& ret_rtti = func->get_return_rtti();
                ParamType _type = ret_rtti.get_decayed_hash() == 0 ? ret_rtti.hash_code() : ret_rtti.get_decayed_hash();
                bool bConstPtr = false;

                auto funcSetOutputParam = [&](const std::string& normalName, const zeno::reflect::Any& returnVal) {
                    auto iterOutputObj = m_outputObjs.find(normalName);
                    if (iterOutputObj != m_outputObjs.end()) {
                        iterOutputObj->second.spObject = any_cast<zany>(returnVal);
                    }
                    else {
                        auto iterOutputPrim = m_outputPrims.find(normalName);
                        if (iterOutputPrim != m_outputPrims.end()) {
                            iterOutputPrim->second.result = returnVal;
                        }
                    }
                };

                if (ret_rtti.flags() & TF_IsMultiReturn) {
                    ArrayList<RTTITypeInfo> rets = func->get_multi_return_rtti();
                    
                    std::vector<Any> retVec = any_cast<std::vector<Any>>(res);
                    //TODO: 有一种可能，就是映射的名称只有一部分，会导致大小和位置不好对的上，后续看看怎么处理
                    assert(rets.size() == mappingReturnParams.size() && retVec.size() == rets.size());
                    for (int i = 0; i < mappingReturnParams.size(); i++) {
                        funcSetOutputParam(mappingReturnParams[i], retVec[i]);
                    }
                }
                else if (!mappingReturnParams.empty()){
                    funcSetOutputParam(mappingReturnParams[0], res);
                }

                //从成员变量到输入
                for (zeno::reflect::IMemberField* field : m_pTypebase->get_member_fields()) {
                    if (const zeno::reflect::IRawMetadata* metadata = field->get_metadata()) {
                        if (const zeno::reflect::IMetadataValue* value = metadata->get_value("Role")) {
                            int _role = value->as_int();
                            if (_role == Role_OutputPrimitive || _role == Role_OutputObject) {
                                std::string field_name(field->get_name().c_str());
                                std::string param_name;
                                if (const zeno::reflect::IMetadataValue* value = metadata->get_value("DisplayName"))
                                    param_name = value->as_string();
                                else {
                                    param_name = field_name;
                                }
                                zeno::reflect::Any outputAny = field->get_field_value(this);
                                if (outputAny.has_value()) {
                                    auto iter = m_outputPrims.find(param_name);
                                    if (iter != m_outputPrims.end()) {
                                        iter->second.result = zeno::reflect::move(outputAny);
                                    }
                                    else {
                                        auto iter2 = m_outputObjs.find(param_name);
                                        if (iter2 != m_outputObjs.end())
                                        {
                                            //TODO: need to parse on the param, not only return value.
                                            //iter2->second.spObject = zeno::reflect::any_cast<std::shared_ptr<IObject>>(outputAny);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                break;
            }
        }
    }
#endif
}

void NodeImpl::commit_to_render(UpdateReason reason) {
    render_update_info info;
    info.reason = reason;
    info.uuidpath_node_objkey = m_uuidPath;

    auto& sess = zeno::getSession();
    sess.objsMan->collect_render_update(info);
}

void NodeImpl::registerObjToManager()
{
    for (auto const& [name, param] : m_outputObjs)
    {
        if (param.spObject)
        {
            //if (std::dynamic_pointer_cast<NumericObject>(param.spObject) ||
            //    std::dynamic_pointer_cast<StringObject>(param.spObject)) {
            //    return;
            //}

            if (param.spObject->key().empty())
            {
                //目前节点处所看到的object，都隶属于此节点本身。
                param.spObject->update_key(stdString2zs(m_uuid));
            }

#if 0
            const std::string& key = param.spObject->key();
            assert(!key.empty());
            param.spObject->nodeId = m_uuidPath;

            auto& objsMan = getSession().objsMan;
            std::shared_ptr<INode> spNode = this;
            objsMan->collectingObject(param.spObject, spNode, m_bView);
#endif
        }
    }
}

void NodeImpl::on_link_added_removed(bool bInput, const std::string& paramname, bool bAdded) {
    checkParamsConstrain();
}

void NodeImpl::on_node_about_to_remove() {
    //移除所有引用边的依赖关系
    for (auto& [_, input_param] : m_inputPrims)
    {
        for (const std::shared_ptr<ReferLink>& reflink : input_param.reflinks) {
            if (reflink->source_inparam == &input_param) {
                //当前参数是引用源
                auto& otherLinks = reflink->dest_inparam->reflinks;
                otherLinks.erase(std::remove(otherLinks.begin(), otherLinks.end(), reflink));
                //参数值也改掉吧，把ref(...)改为 inv_ref(...)
                auto otherNode = reflink->dest_inparam->m_wpNode;
                assert(otherNode);

                auto defl = reflink->dest_inparam->defl;
                assert(defl.has_value());
                ParamType type = defl.type().hash_code();
                if (type == gParamType_PrimVariant) {
                    PrimVar& var = any_cast<PrimVar>(defl);
                    std::visit([&](auto& arg) {
                        using T = std::decay_t<decltype(arg)>;
                        if constexpr (std::is_same_v<T, std::string>) {
                            auto iter = arg.find("ref(");
                            if (iter != std::string::npos) {
                                arg.replace(arg.find("ref("), 4, "ref_not_exist(");
                            }
                        }
                    }, var);
                    otherNode->update_param(reflink->dest_inparam->name, var);
                }
                else if (type == gParamType_VecEdit) {
                    vecvar vec = any_cast<vecvar>(defl);
                    for (auto& elem : vec) {
                        std::visit([&](auto& arg) {
                            using T = std::decay_t<decltype(arg)>;
                            if constexpr (std::is_same_v<T, std::string>) {
                                auto iter = arg.find("ref(");
                                if (iter != std::string::npos) {
                                    arg.replace(iter, 4, "ref_not_exist(");
                                }
                            }
                        }, elem);
                    }
                    otherNode->update_param(reflink->dest_inparam->name, vec);
                }

                if (otherNode)
                    otherNode->mark_dirty(true);
            }
            else {
                //当前参数引用了别的节点参数
                auto& otherLinks = reflink->source_inparam->reflinks;
                otherLinks.erase(std::remove(otherLinks.begin(), otherLinks.end(), reflink));
                auto otherNode = reflink->source_inparam->m_wpNode;
                if (otherNode)
                    otherNode->mark_dirty(true);
            }
        }
        input_param.reflinks.clear();
    }
    for (auto& [_, output_obj] : m_outputObjs)
    {
        for (const std::shared_ptr<ReferLink>& reflink : output_obj.reflinks) {
            if (reflink->source_inparam == &output_obj) {
                //当前参数是引用源
                auto& otherLinks = reflink->dest_inparam->reflinks;
                otherLinks.erase(std::remove(otherLinks.begin(), otherLinks.end(), reflink));

                auto otherNode = reflink->dest_inparam->m_wpNode;
                assert(otherNode);
                if (otherNode)
                    otherNode->mark_dirty(true);
            }
        }
    }
}

void NodeImpl::onNodeNameUpdated(const std::string& oldname, const std::string& newname) {
    std::string graphpath = get_graph_path();
    std::string oldpath = graphpath + '/' + oldname;
    std::string newpath = graphpath + '/' + newname;

    //检查所有reflink，将目标参数的引用名称调整一下
    for (const auto& [_, param] : m_inputPrims) {
        for (auto reflink : param.reflinks) {
            if (reflink->dest_inparam != &param) {
                //直接修改dest_inparam->defl.
                bool bUpdate = false;

                auto fUpdateParamDefl = [oldpath, newpath, graphpath, &bUpdate](std::string& arg) {
                    auto matchs = zeno::getReferPath(arg);
                    for (const auto& str : matchs)
                    {
                        std::string absolutePath = zeno::absolutePath(graphpath, str);
                        if (absolutePath.find(oldpath) != std::string::npos)
                        {
                            std::regex num_rgx("[0-9]+");
                            //如果是数字，需要将整个refer替换
                            if (std::regex_match(newpath, num_rgx))
                            {
                                arg = newpath;
                                bUpdate = true;
                                break;
                            }
                            else
                            {
                                std::regex pattern(oldpath);
                                std::string format = regex_replace(absolutePath, pattern, newpath);
                                //relative path
                                if (absolutePath != str)
                                {
                                    format = zeno::relativePath(graphpath, format);
                                }
                                std::regex rgx(str);
                                arg = regex_replace(arg, rgx, format);
                            }
                            bUpdate = true;
                        }
                    }
                };

                zeno::reflect::Any adjustParamVal = reflink->dest_inparam->defl;

                assert(adjustParamVal.has_value());
                ParamType type = adjustParamVal.type().hash_code();
                if (type == zeno::types::gParamType_PrimVariant) {
                    PrimVar& var = zeno::reflect::any_cast<PrimVar>(adjustParamVal);
                    std::visit([&](auto& arg) {
                        using T = std::decay_t<decltype(arg)>;
                        if constexpr (std::is_same_v<T, std::string>) {
                            fUpdateParamDefl(arg);
                        }
                        else {
                            assert(false);
                            zeno::log_warn("error param type");
                        }
                        }, var);
                    if (bUpdate) {
                        adjustParamVal = zeno::reflect::move(var);
                    }
                }
                else if (type == zeno::types::gParamType_VecEdit) {
                    vecvar var = zeno::reflect::any_cast<vecvar>(adjustParamVal);
                    for (PrimVar& elem : var)
                    {
                        std::visit([&](auto& arg) {
                            using T = std::decay_t<decltype(arg)>;
                            if constexpr (std::is_same_v<T, std::string>) {
                                fUpdateParamDefl(arg);
                            }
                            }, elem);
                    }
                    if (bUpdate) {
                        adjustParamVal = zeno::reflect::move(var);
                    }
                }
                else {
                    assert(false);
                    zeno::log_error("unknown param type of refer param");
                }

                if (bUpdate) {
                    auto spDestNode = reflink->dest_inparam->m_wpNode;
                    spDestNode->update_param(reflink->dest_inparam->name, adjustParamVal);
                }
            }
        }
    }
}

void NodeImpl::constructReference(const std::string& param_name) {
    auto iter = m_inputPrims.find(param_name);
    if (iter == m_inputPrims.end())
        return;

    const Any& param_defl = iter->second.defl;
    initReferLinks(&iter->second);
}

void NodeImpl::initReferLinks(PrimitiveParam* target_param) {
    std::set<std::pair<std::string, std::string>> refSources = resolveReferSource(target_param->defl);
    auto newAdded = refSources;

    for (auto iter = target_param->reflinks.begin(); iter != target_param->reflinks.end(); )
    {
        bool bExist = false;
        std::shared_ptr<ReferLink> spRefLink = (*iter);
        CoreParam* remote_source = spRefLink->source_inparam;
        assert(remote_source);
        if (remote_source == target_param) {
            iter++;
            continue;
        }

        //查看当前link在新的集合里是否还存在。
        for (const auto& [source_node_uuidpath, source_param] : refSources)
        {
            auto spSrcNode = remote_source->m_wpNode;
            if (spSrcNode->get_uuid_path() == source_node_uuidpath &&
                remote_source->name == source_param) {
                //已经有了
                bExist = true;
                newAdded.erase({ source_node_uuidpath, source_param });
                break;
            }
        }

        if (bExist) {
            iter++;
        }
        else {
            iter = target_param->reflinks.erase(iter);
            auto& other_links = remote_source->reflinks;
            other_links.erase(std::remove(other_links.begin(), other_links.end(), spRefLink));
        }
    }

    for (const auto& [source_node_uuidpath, source_param] : newAdded)
    {
        NodeImpl* srcNode = getSession().getNodeByUuidPath(source_node_uuidpath);
        auto iterSrcParam = srcNode->m_inputPrims.find(source_param);
        if (iterSrcParam != srcNode->m_inputPrims.end()) {
            PrimitiveParam& srcparam = iterSrcParam->second;
            if (&srcparam != target_param)  //排除直接引用自己的情况
            {
                //构造reflink
                std::shared_ptr<ReferLink> reflink = std::make_shared<ReferLink>();
                reflink->source_inparam = &srcparam;
                reflink->dest_inparam = target_param;
                target_param->reflinks.push_back(reflink);
                srcparam.reflinks.push_back(reflink);
            }
        } else {
            auto iterSrcObj = srcNode->m_outputObjs.find(source_param);
            if (iterSrcObj != srcNode->m_outputObjs.end()) {
                ObjectParam& srcObj = iterSrcObj->second;
                //构造reflink
                std::shared_ptr<ReferLink> reflink = std::make_shared<ReferLink>();
                reflink->source_inparam = &srcObj;
                reflink->dest_inparam = target_param;
                target_param->reflinks.push_back(reflink);
                srcObj.reflinks.push_back(reflink);
            }
        }
    }
}

std::set<std::pair<std::string, std::string>> NodeImpl::resolveReferSource(const Any& param_defl) {

    std::set<std::pair<std::string, std::string>> refSources;
    std::vector<std::string> refSegments;

    ParamType deflType = param_defl.type().hash_code();
    if (deflType == zeno::types::gParamType_String) {
        const std::string& param_text = zeno::any_cast_to_string(param_defl);
        if (param_text.find("ref") != std::string::npos) {
            refSegments.push_back(param_text);
        }
    }
    else if (deflType == zeno::types::gParamType_PrimVariant) {
        zeno::PrimVar var = zeno::reflect::any_cast<zeno::PrimVar>(param_defl);
        if (!std::holds_alternative<std::string>(var)) {
            return refSources;
        }
        std::string param_text = std::get<std::string>(var);
        if (param_text.find("ref") != std::string::npos) {
            refSegments.push_back(param_text);
        }
    }
    else if (deflType == zeno::types::gParamType_VecEdit) {
        const zeno::vecvar& vec = zeno::reflect::any_cast<zeno::vecvar>(param_defl);
        for (const zeno::PrimVar& elem : vec) {
            if (!std::holds_alternative<std::string>(elem)) {
                continue;
            }
            std::string param_text = std::get<std::string>(elem);
            if (param_text.find("ref") != std::string::npos) {
                refSegments.push_back(param_text);
            }
        }
    }

    if (refSegments.empty())
        return refSources;

    auto namePath = get_path();

    //需要用zfxparser直接parse出所有引用信息
    GlobalError err;
    zeno::GraphException::catched([&] {
        auto& funcMgr = zeno::getSession().funcManager;
        ZfxContext ctx;
        ctx.spNode = this;
        for (auto param_text : refSegments)
        {
            std::string code = param_text;
            if (param_text.find('\n') == std::string::npos && param_text.back() != ';') {
                code = param_text + ';';    //方便通过zfx语句编译
            }

            ZfxContext ctx;
            ctx.spNode = this;
            ctx.spObject = nullptr;
            ctx.code = code;
            ZfxExecute zfx(code, &ctx);
            int ret = zfx.parse();
            if (ret == 0)
            {
                ctx.code = code;
                std::shared_ptr<ZfxASTNode> astRoot = zfx.getASTResult();
                std::set<std::pair<std::string, std::string>> paths =
                    funcMgr->getReferSources(astRoot, &ctx);
                if (!paths.empty()) {
                    refSources.insert(paths.begin(), paths.end());
                }
            }
        }
    }, err);
    return refSources;
}

std::shared_ptr<DictObject> NodeImpl::processDict(ObjectParam* in_param, CalcContext* pContext) {
    std::shared_ptr<DictObject> spDict;
    //连接的元素是list还是list of list的规则，参照Graph::addLink下注释。
    bool bDirecyLink = false;
    const auto& inLinks = in_param->links;
#if 0
    if (inLinks.size() == 1)
    {
        std::shared_ptr<ObjectLink> spLink = inLinks.front();
        auto out_param = spLink->fromparam;
        std::shared_ptr<INode> outNode = out_param->m_wpNode;
        assert(outNode);

        if (out_param->type == in_param->type && spLink->tokey.empty()) //根据Graph::addLink规则，类型相同且无key视为直连
        {
            bDirecyLink = true;
            if (outNode->is_dirty()) {
                GraphException::translated([&] {
                    outNode->doApply(pContext);
                    }, outNode.get());
            }
            auto outResult = outNode->get_output_obj(out_param->name);
            assert(outResult);
            assert(out_param->type == gParamType_Dict);

            //规则如普通节点，都是直接拷贝
#if 0
            if (in_param->socketType == Socket_Owning) {
                spDict = std::dynamic_pointer_cast<DictObject>(outResult->move_clone());
            }
            else if (in_param->socketType == Socket_ReadOnly) {
                spDict = std::dynamic_pointer_cast<DictObject>(outResult);
            }
            else if (in_param->socketType == Socket_Clone)
#endif
            {
                //里面的元素也要clone
                std::shared_ptr<DictObject> outDict = std::dynamic_pointer_cast<DictObject>(outResult);
                spDict = std::dynamic_pointer_cast<DictObject>(outDict->clone_by_key(m_uuid));
            }
            spDict->update_key(m_uuid);
            return spDict;
        }
    }
#endif
    if (!bDirecyLink)
    {
        std::map<std::string, zany> existObjs;
        if (in_param->spObject) {
            std::shared_ptr<DictObject> spOldDict;
            spOldDict = std::dynamic_pointer_cast<DictObject>(in_param->spObject);
            for (auto& [key, spobj] : spOldDict->lut) {
                std::string skey = zsString2Std(spobj->key());
                existObjs.insert({skey, spobj});
            }
        }

        spDict = std::make_shared<DictObject>();
        for (const auto& spLink : in_param->links)
        {
            const std::string& keyName = spLink->tokey;
            auto out_param = spLink->fromparam;
            zeno::NodeImpl* outNode = out_param->m_wpNode;

            bool is_this_item_dirty = outNode->is_dirty();

            if (is_this_item_dirty) {  //list中的元素是dirty的，重新计算并加入list
                GraphException::translated([&] {
                    outNode->doApply(pContext);
                }, outNode);
            }

            auto outResult = outNode->get_output_obj(out_param->name);
            assert(outResult);
            {
                zany newObj;
                if (auto _spList = std::dynamic_pointer_cast<zeno::ListObject>(outResult)) {
                    newObj = clone_by_key(_spList.get(), m_uuid);
                }
                else if (auto _spDict = std::dynamic_pointer_cast<zeno::DictObject>(outResult)) {
                    newObj = clone_by_key(_spDict.get(), m_uuid);
                }
                else {
                    zeno::String newkey = stdString2zs(m_uuid) + '\\' + outResult->key();
                    newObj = outResult->clone();
                    newObj->update_key(newkey);
                }
                spDict->lut[keyName] = newObj;
                std::string const& new_key = zsString2Std(newObj->key());

                if (is_this_item_dirty) {
                    //需要区分是新的还是旧的，这里先粗暴认为全是新的
                    if (existObjs.find(new_key) != existObjs.end()) {
                        spDict->m_modify.insert(new_key);
                        existObjs.erase(new_key);
                    }
                    else {
                        spDict->m_new_added.insert(new_key);
                    }
                } else {
                    spDict->m_modify.insert(new_key);//需视为modify，否则关闭这个list节点的view时会崩或显示不正常的bug
                }
                existObjs.erase(new_key);
            }
        }
        //剩下没出现的都认为是移除掉了
        std::function<void(zany)> flattenDict = [&flattenDict, &spDict](zany obj) {
            if (auto _spList = std::dynamic_pointer_cast<ListObject>(obj)) {
                for (int i = 0; i < _spList->m_impl->size(); ++i) {
                    flattenDict(_spList->m_impl->get(i));
                }
            } else if (auto _spDict = std::dynamic_pointer_cast<DictObject>(obj)) {
                for (auto& [key, obj] : _spDict->get()) {
                    flattenDict(obj);
                }
            } else {
                spDict->m_new_removed.insert(zsString2Std(obj->key()));
            }
        };
        for (auto& [key, removeobj] : existObjs) {
            flattenDict(removeobj);//提前节需要移除的对象的key全部展平，在GraphicsManager.h中一次性移除，才能正确显示
        }

        spDict->update_key(stdString2zs(m_uuid));
    }
    return spDict;
}

std::shared_ptr<ListObject> NodeImpl::processList(ObjectParam* in_param, CalcContext* pContext) {
    std::shared_ptr<ListObject> spList;
    bool bDirectLink = false;
#if 0
    if (in_param->links.size() == 1)
    {
        std::shared_ptr<ObjectLink> spLink = in_param->links.front();
        auto out_param = spLink->fromparam;
        std::shared_ptr<INode> outNode = out_param->m_wpNode;

        if (out_param->type == in_param->type && spLink->tokey.empty()) {   //根据Graph::addLink规则，类型相同且无key视为直连
            bDirectLink = true;

            if (outNode->is_dirty()) {
                GraphException::translated([&] {
                    outNode->doApply(pContext);
                    }, outNode.get());
            }
            auto outResult = outNode->get_output_obj(out_param->name);
            assert(outResult);
            assert(out_param->type == gParamType_List);

#if 0
            if (in_param->socketType == Socket_Owning) {
                spList = std::dynamic_pointer_cast<ListObject>(outResult->move_clone());
            }
            else if (in_param->socketType == Socket_ReadOnly) {
                spList = std::dynamic_pointer_cast<ListObject>(outResult);
            }
            else if (in_param->socketType == Socket_Clone)
#endif
            {
                //里面的元素也要clone
                std::shared_ptr<ListObject> outList = std::dynamic_pointer_cast<ListObject>(outResult);
                spList = std::dynamic_pointer_cast<ListObject>(outList->clone_by_key(m_uuid));
#if 0
                spList = create_ListObject();
                std::shared_ptr<ListObject> outList = std::dynamic_pointer_cast<ListObject>(outResult);
                for (int i = 0; i < outList->size(); i++) {
                    //后续要考虑key的问题
                    spList->push_back(outList->get(i)->clone());
                }
#endif
            }
            spList->update_key(m_uuid);
        }
    }
#endif
    if (!bDirectLink)
    {
        spList = create_ListObject();

        std::map<std::string, zany> existObjs;
        if (in_param->spObject) {
            auto spOldList = dynamic_cast<ListObject*>(in_param->spObject.get());
            for (auto spobj : spOldList->m_impl->get()) {
                existObjs.insert({zsString2Std(spobj->key()), spobj});
            }
        }

        int indx = 0;
        for (const auto& spLink : in_param->links)
        {
            //list的情况下，keyName是不是没意义，顺序怎么维持？
            auto out_param = spLink->fromparam;
            NodeImpl* outNode = out_param->m_wpNode;
            bool is_this_item_dirty = outNode->is_dirty();
            if (is_this_item_dirty) {  //list中的元素是dirty的，重新计算并加入list
                GraphException::translated([&] {
                    outNode->doApply(pContext);
                }, outNode);
            }
            auto outResult = outNode->get_output_obj(out_param->name);
            assert(outResult);
            {
                zany newObj;
                if (auto _spList = dynamic_cast<zeno::ListObject*>(outResult.get())) {
                    newObj = clone_by_key(_spList, m_uuid);
                }
                else if (auto _spDict = dynamic_cast<zeno::DictObject*>(outResult.get())) {
                    newObj = clone_by_key(_spDict, m_uuid);
                }
                else {
                    auto newkey = stdString2zs(m_uuid) + '\\' + outResult->key();
                    if (dynamic_cast<zeno::PrimitiveObject*>(outResult.get())) {
                        newObj = outResult;
                        out_param->spObject.reset();
                        outNode->mark_dirty(true);
                    }
                    else {
                        newObj = outResult->clone();
                        newObj->update_key(newkey);
                    }
                }
                spList->m_impl->push_back(newObj);

                //需要区分是新的还是旧的，这里先粗暴认为全是新的
                std::string const& new_key = zsString2Std(newObj->key());
                if (is_this_item_dirty) {
                    //需要区分是新的还是旧的，这里先粗暴认为全是新的
                    if (existObjs.find(new_key) != existObjs.end()) {
                        spList->m_impl->m_modify.insert(new_key);
                        existObjs.erase(new_key);
                    }
                    else {
                        spList->m_impl->m_new_added.insert(new_key);
                    }
                } else {
                    spList->m_impl->m_modify.insert(new_key);//需视为modify，否则关闭这个list节点的view时会崩或显示不正常的bug
                }
                existObjs.erase(new_key);
            }
        }
        //剩下没出现的都认为是移除掉了
        std::function<void(zany)> flattenList = [&flattenList, &spList](zany obj) {
            if (auto _spList = std::dynamic_pointer_cast<zeno::ListObject>(obj)) {
                for (int i = 0; i < _spList->m_impl->size(); ++i) {
                    flattenList(_spList->m_impl->get(i));
                }
            } else if (auto _spDict = std::dynamic_pointer_cast<zeno::DictObject>(obj)) {
                for (auto& [key, obj] : _spDict->get()) {
                    flattenList(obj);
                }
            } else {
                spList->m_impl->m_new_removed.insert(zsString2Std(obj->key()));
            }
        };
        for (auto& [key, removeobj] : existObjs) {
            flattenList(removeobj);//提前节需要移除的对象的key全部展平，在GraphicsManager.h中一次性移除，才能正确显示
        }
        spList->update_key(stdString2zs(m_uuid));
    }
    return spList;
}

zeno::reflect::Any NodeImpl::processPrimitive(PrimitiveParam* in_param)
{
    //没有连线的情况，这时候直接取defl作为result。
    if (!in_param) {
        return Any();
    }

    int frame = getGlobalState()->getFrameId();

    const ParamType type = in_param->type;
    const auto& defl = in_param->defl;
    if (type == gParamType_Heatmap && !defl.has_value()) {
        //先跳过heatmap
        return Any();
    }
    assert(defl.has_value());
    zeno::reflect::Any result = defl;
    ParamType editType = defl.type().hash_code();

    switch (type) {
    case gParamType_Int:
    case gParamType_Float:
    {
        if (editType == gParamType_PrimVariant) {
            zeno::PrimVar var = any_cast<zeno::PrimVar>(defl);
            result = std::visit([=](auto&& arg)->Any {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, int> || std::is_same_v<T, float>) {
                    return arg;
                }
                else if constexpr (std::is_same_v<T, std::string>) {
                    float res = resolve(arg, type);
                    return (type == gParamType_Int) ? zeno::reflect::make_any<int>(res) : 
                        zeno::reflect::make_any<float>(res);
                }
                else if constexpr (std::is_same_v<T, CurveData>) {
                    int frame = getGlobalState()->getFrameId();
                    return arg.eval(frame);
                }
                else {
                    throw makeError<UnimplError>();
                }
            }, var);
        }
        else if (editType == gParamType_Int) {
            //目前所有defl都是以PrimVariant的方式储存，暂时不会以本值类型储存
            assert(false);
        }
        else if (editType == gParamType_Float) {
            assert(false);
        }
        else {
            assert(false);
        }
        break;
    }
    case zeno::types::gParamType_Bool:
    {
        //Bool值暂不支持控件编写表达式，因此直接取值
        assert(editType == gParamType_Bool);
        result = std::move(defl);
        break;
    }
    case zeno::types::gParamType_String:
    {
        //TODO: format string as formula
        break;
    }
    case gParamType_Vec2f:
    case gParamType_Vec2i:
    case gParamType_Vec3f:
    case gParamType_Vec3i:
    case gParamType_Vec4f:
    case gParamType_Vec4i:
    {
        assert(gParamType_VecEdit == editType);
        zeno::vecvar editvec = any_cast<zeno::vecvar>(defl);
        std::vector<float> vec;
        for (int i = 0; i < editvec.size(); i++)
        {
            float res = std::visit([=](auto&& arg)->float {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, int> || std::is_same_v<T, float>) {
                    return arg;
                }
                else if constexpr (std::is_same_v<T, std::string>) {
                    return resolve(arg, type);
                }
                else if constexpr (std::is_same_v<T, CurveData>) {
                    int frame = getGlobalState()->getFrameId();
                    return arg.eval(frame);
                }
                else {
                    throw makeError<UnimplError>();
                }
            }, editvec[i]);
            vec.push_back(res);
        }
        if (type == gParamType_Vec2f)       result = zeno::vec2f(vec[0], vec[1]);
        else if (type == gParamType_Vec2i)  result = zeno::vec2i(vec[0], vec[1]);
        else if (type == gParamType_Vec3f)  result = zeno::vec3f(vec[0], vec[1], vec[2]);
        else if (type == gParamType_Vec3i)  result = zeno::vec3i(vec[0], vec[1], vec[2]);
        else if (type == gParamType_Vec4f)  result = zeno::vec4f(vec[0], vec[1], vec[2], vec[3]);
        else if (type == gParamType_Vec4i)  result = zeno::vec4i(vec[0], vec[1], vec[2], vec[3]);
        break;
    }
    //case zeno::types::gParamType_Heatmap:
    //{
    //    //TODO: heatmap的结构体定义.
    //    //if (std::holds_alternative<std::string>(defl))
    //    //    result = zeno::parseHeatmapObj(std::get<std::string>(defl));
    //    break;
    //}
    //这里指的是基础类型的List/Dict.
    case gParamType_List:
    {
        //TODO: List现在还没有ui支持，而且List是泛型容器，对于非Literal值不好设定默认值。
        break;
    }
    case gParamType_Dict:
    {
        break;
    }
    case gParamType_Shader:
    {
        throw makeError<UnimplError>("no defl value supported on the param with type `gParamType_Shader`, please connect the link by outside");
#if 0
        if (editType == gParamType_PrimVariant) {
            zeno::PrimVar var = any_cast<zeno::PrimVar>(defl);
            result = std::visit([=](auto&& arg)->Any {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, int> || std::is_same_v<T, float>) {
                    return arg;
                }
                else if constexpr (std::is_same_v<T, std::string>) {
                    float res = resolve(arg, type);
                    return (type == gParamType_Int) ? zeno::reflect::make_any<int>(res) :
                        zeno::reflect::make_any<float>(res);
                }
                else if constexpr (std::is_same_v<T, CurveData>) {
                    int frame = getGlobalState()->getFrameId();
                    return arg.eval(frame);
                }
                else {
                    throw makeError<UnimplError>();
                }
            }, var);
        }
        else if (editType == gParamType_Int) {
            result = defl;
        }
        else if (editType == gParamType_Float) {
            result = defl;
        }
        break;
#endif
    }
    }
    return result;
}

bool NodeImpl::receiveOutputObj(ObjectParam* in_param, NodeImpl* outNode, ObjectParam* out_param) {

    //在此版本里，只有克隆，每个对象只有一个节点关联，虽然激进，但可以充分测试属性数据共享在面对
    //内存暴涨时的冲击，能优化到什么程度

    bool bCloned = true;
    //如果outputobj是 PrimitiveObject，则不走拷贝，而是move，如果List和Dict含有哪怕只有一个PrimitiveObj，也不走拷贝
    if (auto prim = std::dynamic_pointer_cast<PrimitiveObject>(out_param->spObject)) {
        bCloned = false;
    }
    else if (auto lst = std::dynamic_pointer_cast<ListObject>(out_param->spObject)) {
        if (ListHasPrimObj(lst.get())) {
            bCloned = false;
        }
    }
    else if (auto dict = std::dynamic_pointer_cast<DictObject>(out_param->spObject)) {
        if (DictHasPrimObj(dict.get())) {
            bCloned = false;
        }
    }

    if (bCloned) {
        in_param->spObject = out_param->spObject->clone();
    }
    else {
        in_param->spObject = out_param->spObject;
        out_param->spObject.reset();
        outNode->mark_dirty(true);
    }

    if (auto splist = std::dynamic_pointer_cast<ListObject>(in_param->spObject)) {
        update_list_root_key(splist.get(), m_uuidPath);
    } else if (auto spdict = std::dynamic_pointer_cast<DictObject>(in_param->spObject)) {
        update_dict_root_key(spdict.get(), m_uuidPath);
    } else {
        in_param->spObject->update_key(stdString2zs(m_uuidPath));
    }
#if 0
    if (in_param->socketType == Socket_Clone) {
        in_param->spObject = outputObj->clone();
    }
    else if (in_param->socketType == Socket_Owning) {
        in_param->spObject = outputObj->move_clone();
        assert(outNode);
        outNode->mark_dirty(true);
    }
    else if (in_param->socketType == Socket_ReadOnly) {
        in_param->spObject = outputObj;
        //TODO: readonly property on object.
    }
    else if (in_param->bWildcard) {
        in_param->spObject = outputObj;
    }
#endif
    return true;
}

bool NodeImpl::requireInput(std::string const& ds, CalcContext* pContext) {
    // 目前假设输入对象和输入数值，不能重名（不难实现，老节点直接改）。
    auto iter = m_inputObjs.find(ds);
    if (iter != m_inputObjs.end()) {
        ObjectParam* in_param = &(iter->second);
        if (in_param->links.empty()) {
            //节点如果定义了对象，但没有边连上去，是否要看节点apply如何处理？
            //FIX: 没有边的情况要清空掉对象，否则apply以为这个参数连上了对象
            in_param->spObject.reset();
        }
        else {
            switch (in_param->type)
            {
                case gParamType_Dict:
                {
                    std::shared_ptr<DictObject> outDict = processDict(in_param, pContext);
                    in_param->spObject = outDict;
                    break;
                }
                case gParamType_List:
                {
                    std::shared_ptr<ListObject> outList = processList(in_param, pContext);
                    in_param->spObject = outList;
                    break;
                }
                case gParamType_Curve:
                {
                    //Curve要视作Object，因为整合到variant太麻烦，只要对于最原始的MakeCurve节点，以字符串（储存json）作为特殊类型即可。
                }
                default:
                {
                    if (in_param->links.size() == 1)
                    {
                        std::shared_ptr<ObjectLink> spLink = *(in_param->links.begin());
                        ObjectParam* out_param = spLink->fromparam;
                        auto outNode = out_param->m_wpNode;

                        GraphException::translated([&] {
                            outNode->doApply(pContext);
                        }, outNode);

                        if (out_param->spObject)
                        {
                            receiveOutputObj(in_param, outNode, out_param);
                        }
                    }
                }
            }
        }
    }
    else {
        auto iter2 = m_inputPrims.find(ds);
        if (iter2 != m_inputPrims.end()) {
            PrimitiveParam* in_param = &iter2->second;
            if (in_param->links.empty()) {

                std::list<std::shared_ptr<ReferLink>> depRefLinks;
                for (auto reflink : in_param->reflinks) {
                    if (reflink->source_inparam != in_param) {
                        depRefLinks.push_back(reflink);
                    }
                }

                if (!depRefLinks.empty()) {
                    for (auto reflink : depRefLinks) {
                        assert(reflink->source_inparam);
                        auto spSrcNode = reflink->source_inparam->m_wpNode;
                        assert(spSrcNode);
                        Graph* spSrcGraph = spSrcNode->m_pGraph;
                        assert(spSrcGraph);
                        //NOTE in 2025/3/25: 还是apply引用源，至于本节点参数循环引用的问题，走另外的路线
                        if (spSrcNode == this) {
                            //引用自身的参数，直接拿defl，因为这种情况绝大多数是固定值，没必要执行计算，比如pos引用size的数据
                            spSrcNode->doApply_Parameter(reflink->source_inparam->name, pContext);
                        }
                        else {
                            spSrcNode->doApply(pContext);
                        }
                    }
                }

                in_param->result = processPrimitive(in_param);
                //旧版本的requireInput指的是是否有连线，如果想兼容旧版本，这里可以返回false，但使用量不多，所以就修改它的定义。
            }
            else {
                if (in_param->links.size() == 1) {
                    std::shared_ptr<PrimitiveLink> spLink = *in_param->links.begin();
                    auto outNode = spLink->fromparam->m_wpNode;

                    GraphException::translated([&] {
                        outNode->doApply(pContext);
                    }, outNode);
                    //数值基本类型，直接复制。
                    //注意：这里并不会检查类型，因为有一些特殊情况，比如Shader节点的连接，是允许将Shader类型连到数值输入里的（ShaderFinalize)
                    //这样，检查的工作就交给了节点本身（INode)。
                    in_param->result = spLink->fromparam->result;
                }
            }
        } else {
            return false;
        }
    }
    return true;
}

void NodeImpl::doOnlyApply() {
    apply();
}

void NodeImpl::clear() {
    for (auto& [key, param] : m_inputObjs) {
        param.spObject.reset();
    }
    for (auto& [key, param] : m_outputObjs) {
        param.spObject.reset();
    }
    for (auto& [key, param] : m_inputPrims) {
        param.result.reset();
    }
    for (auto& [key, param] : m_outputPrims) {
        param.result.reset();
    }
}

void NodeImpl::doApply_Parameter(std::string const& name, CalcContext* pContext) {
    if (!m_dirty) {
        return;
    }

    std::string uuid_path = get_uuid_path() + "/" + name;
    if (pContext->uuid_node_params.find(uuid_path) != pContext->uuid_node_params.end()) {
        throw makeError<UnimplError>("cycle reference occurs when refer paramters!");
    }

    scope_exit scope_apply_param([&]() { pContext->uuid_node_params.erase(uuid_path); });
    pContext->uuid_node_params.insert(uuid_path);

    requireInput(name, pContext);
}

void NodeImpl::bypass() {
    //在preAppy拷贝了对象以后再直接赋值给output，还有一种方法是不拷贝，直接把上一个节点的输出给到这里的输出。

    //找到输入和输出的唯一object(如果输入有两个，并且有一个有连线，是否采纳连线这个？）
    //不考虑数值类型的输出
    if (m_outputObjs.empty() || m_inputObjs.empty()) {
        throw makeError<UnimplError>("there is not matched input and output object when mute button is on");
    }
    ObjectParam& input_objparam = m_inputObjs.begin()->second;
    ObjectParam& output_objparam = m_outputObjs.begin()->second;
    if (input_objparam.type != output_objparam.type) {
        throw makeError<UnimplError>("the input and output type is not matched, when the mute button is on");
    }
    output_objparam.spObject = input_objparam.spObject;
}

void NodeImpl::doApply(CalcContext* pContext) {

    if (!m_dirty)
        return;

    assert(pContext);
    std::string uuid_path = get_uuid_path();
    if (pContext->visited_nodes.find(uuid_path) != pContext->visited_nodes.end()) {
        throw makeError<UnimplError>("cycle reference occurs!");
    }
    pContext->visited_nodes.insert(uuid_path);
    scope_exit spUuidRecord([=] {pContext->visited_nodes.erase(uuid_path); });

#if 0
    if (m_name == "locate" && pContext->curr_iter == 1) {
        int j;
        j = 0;
    }
#endif

    if (m_nodecls == "TimeShift") {
        preApplyTimeshift(pContext);
    } else if (m_nodecls == "ForEachEnd") {
        preApply_Primitives(pContext);
    } else {
        preApply(pContext);
    }

    if (zeno::getSession().is_interrupted()) {
        throw makeError<InterruputError>(m_uuidPath);
    }

    log_debug("==> enter {}", m_name);
    {
#ifdef ZENO_BENCHMARKING
        Timer _(m_name);
#endif
        if (m_mute) {
            bypass();
        }
        else {
            reportStatus(true, Node_Running);
            if (m_nodecls == "ForEachEnd") {
                reflectForeach_apply(pContext);
            } else {
                apply();
            }
        }
    }
    log_debug("==> leave {}", m_name);

    //DopNetwork
    if (DopNetwork* dop = dynamic_cast<DopNetwork*>(this)) {
        reportStatus(true, Node_Running);
        registerObjToManager();
        reportStatus(true, Node_DirtyReadyToRun);
    } else {
        //if (m_nodecls == "ForEachEnd") {
        //    reportStatus(true, Node_Running);
        //    registerObjToManager();
        //} else {
            registerObjToManager();
            reportStatus(false, Node_RunSucceed);
        //}
    }
    if (m_bView) {
        commit_to_render(Update_Reconstruct);
    }
}

CommonParam NodeImpl::get_input_param(std::string const& name, bool* bExist) {
    auto primparam = get_input_prim_param(name, bExist);
    if (bExist && *bExist)
        return primparam;
    auto objparam = get_input_obj_param(name, bExist);
    if (bExist && *bExist)
        return objparam;
    if (bExist)
        *bExist = false;
}

CommonParam NodeImpl::get_output_param(std::string const& name, bool* bExist) {
    auto primparam = get_output_prim_param(name, bExist);
    if (bExist && *bExist)
        return primparam;
    auto objparam = get_output_obj_param(name, bExist);
    if (bExist && *bExist)
        return objparam;
    if (bExist)
        *bExist = false;
}

ObjectParams NodeImpl::get_input_object_params() const
{
    ObjectParams params;
    for (auto& [name, spObjParam] : m_inputObjs)
    {
        ParamObject obj;
        for (auto linkInfo : spObjParam.links) {
            obj.links.push_back(getEdgeInfo(linkInfo));
        }
        obj.name = name;
        obj.type = spObjParam.type;
        obj.bInput = true;
        obj.socketType = spObjParam.socketType;
        obj.wildCardGroup = spObjParam.wildCardGroup;
        //obj.prop = ?
        params.push_back(obj);
    }
    return params;
}

ObjectParams NodeImpl::get_output_object_params() const
{
    ObjectParams params;
    for (auto& [name, spObjParam] : m_outputObjs)
    {
        ParamObject obj;
        for (auto linkInfo : spObjParam.links) {
            obj.links.push_back(getEdgeInfo(linkInfo));
        }
        obj.name = name;
        obj.type = spObjParam.type;
        obj.bInput = false;
        obj.socketType = spObjParam.socketType;
        obj.wildCardGroup = spObjParam.wildCardGroup;
        //obj.prop = ?
        params.push_back(obj);
    }
    return params;
}

PrimitiveParams NodeImpl::get_input_primitive_params() const {
    //TODO: deprecated node.
    PrimitiveParams params;
    for (auto& [name, spParamObj] : m_inputPrims) {
        ParamPrimitive param;
        param.bInput = true;
        param.name = name;
        param.type = spParamObj.type;
        param.control = spParamObj.control;
        param.ctrlProps = spParamObj.ctrlProps;
        param.defl = spParamObj.defl;
        param.bSocketVisible = spParamObj.bSocketVisible;
        for (auto spLink : spParamObj.links) {
            param.links.push_back(getEdgeInfo(spLink));
        }
        param.socketType = spParamObj.socketType;
        param.wildCardGroup = spParamObj.wildCardGroup;
        params.push_back(param);
    }
    return params;
}

PrimitiveParams NodeImpl::get_output_primitive_params() const {
    PrimitiveParams params;
    for (auto& [name, spParamObj] : m_outputPrims) {
        ParamPrimitive param;
        param.bInput = false;
        param.name = name;
        param.type = spParamObj.type;
        param.control = NullControl;
        param.defl = spParamObj.defl;
        for (auto spLink : spParamObj.links) {
            param.links.push_back(getEdgeInfo(spLink));
        }
        param.socketType = spParamObj.socketType;
        param.wildCardGroup = spParamObj.wildCardGroup;
        params.push_back(param);
    }
    return params;
}

ParamPrimitive NodeImpl::get_input_prim_param(std::string const& name, bool* pExist) const {
    ParamPrimitive param;
    auto iter = m_inputPrims.find(name);
    if (iter != m_inputPrims.end()) {
        auto& paramPrim = iter->second;
        param = paramPrim.exportParam();
        if (pExist)
            *pExist = true;
    }
    else {
        if (pExist)
            *pExist = false;
    }
    return param;
}

ParamObject NodeImpl::get_input_obj_param(std::string const& name, bool* pExist) const {
    ParamObject param;
    auto iter = m_inputObjs.find(name);
    if (iter != m_inputObjs.end()) {
        auto& paramObj = iter->second;
        param = paramObj.exportParam();
        if (pExist)
            *pExist = true;
    }
    else {
        if (pExist)
            *pExist = false;
    }
    return param;
}

ParamPrimitive NodeImpl::get_output_prim_param(std::string const& name, bool* pExist) const {
    ParamPrimitive param;
    auto iter = m_outputPrims.find(name);
    if (iter != m_outputPrims.end()) {
        auto& paramPrim = iter->second;
        param = paramPrim.exportParam();
        if (pExist)
            *pExist = true;
    }
    else {
        if (pExist)
            *pExist = false;
    }
    return param;
}

ParamObject NodeImpl::get_output_obj_param(std::string const& name, bool* pExist) const {
    ParamObject param;
    auto iter = m_outputObjs.find(name);
    if (iter != m_outputObjs.end()) {
        auto& paramObj = iter->second;
        param = paramObj.exportParam();
        if (pExist)
            *pExist = true;
    }
    else {
        if (pExist)
            *pExist = false;
    }
    return param;
}

zeno::reflect::Any NodeImpl::get_defl_value(std::string const& name) {
    //向量情况也挺麻烦的，因为可能存在公式
    ParamPrimitive param;
    auto iter = m_inputPrims.find(name);
    if (iter != m_inputPrims.end()) {
        zeno::reflect::Any defl = iter->second.defl;
        //不支持取公式，因为公式要引发计算，很麻烦
        convertToOriginalVar(defl, iter->second.type);
        return defl;
    }else{
        return zeno::reflect::Any();
    }
}

zeno::reflect::Any NodeImpl::get_param_result(std::string const& name) {
    const PrimitiveParam& param = safe_at(m_inputPrims, name, "prim param");
    return param.result;
}

bool NodeImpl::add_input_prim_param(ParamPrimitive param) {
    if (m_inputPrims.find(param.name) != m_inputPrims.end()) {
        return false;
    }
    PrimitiveParam sparam;
    sparam.bInput = true;
    sparam.control = param.control;
    sparam.defl = param.defl;
    convertToEditVar(sparam.defl, param.type);
    sparam.m_wpNode = this;
    sparam.name = param.name;
    sparam.socketType = param.socketType;
    sparam.type = param.type;
    sparam.ctrlProps = param.ctrlProps;
    sparam.bSocketVisible = param.bSocketVisible;
    sparam.wildCardGroup = param.wildCardGroup;
    sparam.sockprop = param.sockProp;
    //sparam.bInnerParam = param.bInnerParam;
    sparam.constrain = param.constrain;
    m_inputPrims.insert(std::make_pair(param.name, std::move(sparam)));
    return true;
}

bool NodeImpl::add_input_obj_param(ParamObject param) {
    if (m_inputObjs.find(param.name) != m_inputObjs.end()) {
        return false;
    }
    ObjectParam sparam;
    sparam.bInput = true;
    sparam.name = param.name;
    sparam.type = param.type;
    sparam.socketType = Socket_Clone;// param.socketType;
    sparam.m_wpNode = this;
    sparam.wildCardGroup = param.wildCardGroup;
    sparam.constrain = param.constrain;
    m_inputObjs.insert(std::make_pair(param.name, std::move(sparam)));
    return true;
}

bool NodeImpl::add_output_prim_param(ParamPrimitive param) {
    if (m_outputPrims.find(param.name) != m_outputPrims.end()) {
        return false;
    }
    PrimitiveParam sparam;
    sparam.bInput = false;
    sparam.control = param.control;
    sparam.defl = param.defl;
    sparam.m_wpNode = this;
    sparam.name = param.name;
    sparam.socketType = Socket_Clone;// param.socketType;
    sparam.type = param.type;
    sparam.ctrlProps = param.ctrlProps;
    sparam.wildCardGroup = param.wildCardGroup;
    sparam.bSocketVisible = param.bSocketVisible;
    sparam.constrain = param.constrain;
    m_outputPrims.insert(std::make_pair(param.name, std::move(sparam)));
    return true;
}

bool NodeImpl::add_output_obj_param(ParamObject param) {
    if (m_outputObjs.find(param.name) != m_outputObjs.end()) {
        return false;
    }
    ObjectParam sparam;
    sparam.bInput = false;
    sparam.name = param.name;
    sparam.type = param.type;
    sparam.socketType = param.socketType;
    sparam.constrain = param.constrain;
    sparam.m_wpNode = this;
    sparam.wildCardGroup = param.wildCardGroup;
    m_outputObjs.insert(std::make_pair(param.name, std::move(sparam)));
    return true;
}

void NodeImpl::set_result(bool bInput, const std::string& name, zany spObj) {
    if (bInput) {
        auto& param = safe_at(m_inputObjs, name, "");
        param.spObject = spObj;
    }
    else {
        auto& param = safe_at(m_outputObjs, name, "");
        param.spObject = spObj;
    }
}

void NodeImpl::init_object_link(bool bInput, const std::string& paramname, std::shared_ptr<ObjectLink> spLink, const std::string& targetParam) {
    auto iter = bInput ? m_inputObjs.find(paramname) : m_outputObjs.find(paramname);
    if (bInput)
        spLink->toparam = &iter->second;
    else
        spLink->fromparam = &iter->second;
    spLink->targetParam = targetParam;
    iter->second.links.emplace_back(spLink);
}

void NodeImpl::init_primitive_link(bool bInput, const std::string& paramname, std::shared_ptr<PrimitiveLink> spLink, const std::string& targetParam) {
    auto iter = bInput ? m_inputPrims.find(paramname) : m_outputPrims.find(paramname);
    if (bInput)
        spLink->toparam = &iter->second;
    else
        spLink->fromparam = &iter->second;
    spLink->targetParam = targetParam;
    iter->second.links.emplace_back(spLink);
}

bool NodeImpl::isPrimitiveType(bool bInput, const std::string& param_name, bool& bExist) {
    if (bInput) {
        if (m_inputObjs.find(param_name) != m_inputObjs.end()) {
            bExist = true;
            return false;
        }
        else if (m_inputPrims.find(param_name) != m_inputPrims.end()) {
            bExist = true;
            return true;
        }
        bExist = false;
        return false;
    }
    else {
        if (m_outputObjs.find(param_name) != m_outputObjs.end()) {
            bExist = true;
            return false;
        }
        else if (m_outputPrims.find(param_name) != m_outputPrims.end()) {
            bExist = true;
            return true;
        }
        bExist = false;
        return false;
    }
}

std::vector<EdgeInfo> NodeImpl::getLinks() const {
    std::vector<EdgeInfo> remLinks;
    for (const auto& [_, spParam] : m_inputObjs) {
        for (std::shared_ptr<ObjectLink> spLink : spParam.links) {
            remLinks.push_back(getEdgeInfo(spLink));
        }
    }
    for (const auto& [_, spParam] : m_inputPrims) {
        for (std::shared_ptr<PrimitiveLink> spLink : spParam.links) {
            remLinks.push_back(getEdgeInfo(spLink));
        }
    }
    for (const auto& [_, spParam] : m_outputObjs) {
        for (std::shared_ptr<ObjectLink> spLink : spParam.links) {
            remLinks.push_back(getEdgeInfo(spLink));
        }
    }
    for (const auto& [_, spParam] : m_outputPrims) {
        for (std::shared_ptr<PrimitiveLink> spLink : spParam.links) {
            remLinks.push_back(getEdgeInfo(spLink));
        }
    }
    return remLinks;
}

std::vector<EdgeInfo> NodeImpl::getLinksByParam(bool bInput, const std::string& param_name) const {
    std::vector<EdgeInfo> links;

    auto& objects = bInput ? m_inputObjs : m_outputObjs;
    auto& primtives = bInput ? m_inputPrims : m_outputPrims;

    auto iter = objects.find(param_name);
    if (iter != objects.end()) {
        for (auto spLink : iter->second.links) {
            links.push_back(getEdgeInfo(spLink));
        }
    }
    else {
        auto iter2 = primtives.find(param_name);
        if (iter2 != primtives.end()) {
            for (auto spLink : iter2->second.links) {
                links.push_back(getEdgeInfo(spLink));
            }
        }
    }
    return links;
}

bool NodeImpl::updateLinkKey(bool bInput, const zeno::EdgeInfo& edge, const std::string& oldkey, const std::string& newkey)
{
    auto& objects = bInput ? m_inputObjs : m_outputObjs;
    auto iter = objects.find(edge.inParam);
    if (iter != objects.end()) {
        for (auto spLink : iter->second.links) {
            if (auto fromParam = spLink->fromparam) {
                if (auto outnode = fromParam->m_wpNode) {
                    if (outnode->get_name() == edge.outNode && spLink->tokey == oldkey) {   //需outnode和tokey均相同
                        spLink->tokey = newkey;
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

bool NodeImpl::moveUpLinkKey(bool bInput, const std::string& param_name, const std::string& key)
{
    auto& objects = bInput ? m_inputObjs : m_outputObjs;
    auto iter = objects.find(param_name);
    if (iter != objects.end()) {
        for (auto it = iter->second.links.begin(); it != iter->second.links.end(); it++) {
            if ((*it)->tokey == key) {
                auto it_ = std::prev(it);
                std::swap(*it, *it_);
                return true;
            }
        }
    }
    return false;
}

bool NodeImpl::removeLink(bool bInput, const EdgeInfo& edge) {
    //现有规则不允许输入对象和输入数值参数同名，所以不需要bObjLink，而且bObjLink好像只是用于给ui区分而已
    if (bInput) {
        auto iter = m_inputObjs.find(edge.inParam);
        if (iter != m_inputObjs.end()) {
            for (auto spLink : iter->second.links) {
                if (auto outNode = spLink->fromparam->m_wpNode) {
                    if (outNode->get_name() == edge.outNode && spLink->fromparam->name == edge.outParam && spLink->fromkey == edge.outKey) {
                        iter->second.links.remove(spLink);
                        return true;
                    }
                }
            }
        }
        auto iter2 = m_inputPrims.find(edge.inParam);
        if (iter2 != m_inputPrims.end()) {
            for (auto spLink : iter2->second.links) {
                if (auto outNode = spLink->fromparam->m_wpNode) {
                    if (outNode->get_name() == edge.outNode && spLink->fromparam->name == edge.outParam) {
                        iter2->second.links.remove(spLink);
                        return true;
                    }
                }
            }
        }
    }
    else {
        auto iter = m_outputObjs.find(edge.outParam);
        if (iter != m_outputObjs.end())
        {
            for (auto spLink : iter->second.links) {
                if (auto inNode = spLink->toparam->m_wpNode) {
                    if (inNode->get_name() == edge.inNode && spLink->toparam->name == edge.inParam && spLink->tokey == edge.inKey) {
                        iter->second.links.remove(spLink);
                        return true;
                    }
                }
            }
        }

        auto iter2 = m_outputPrims.find(edge.outParam);
        if (iter2 != m_outputPrims.end())
        {
            for (auto spLink : iter2->second.links) {
                if (auto inNode = spLink->toparam->m_wpNode) {
                    if (inNode->get_name() == edge.inNode && spLink->toparam->name == edge.inParam) {
                        iter2->second.links.remove(spLink);
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

std::string NodeImpl::get_viewobject_output_param() const {
    //现在暂时还没有什么标识符用于指定哪个输出口是对应输出view obj的
    //一般都是默认第一个输出obj，暂时这么规定，后续可能用标识符。
    if (m_outputObjs.empty())
        return "";
    return m_outputObjs.begin()->second.name;
}

NodeData NodeImpl::exportInfo() const
{
    NodeData node;
    node.cls = m_nodecls;
    node.name = m_name;
    node.bView = m_bView;
    node.uipos = m_pos;
    //TODO: node type
    if (node.subgraph.has_value())
        node.type = Node_SubgraphNode;
    else
        node.type = Node_Normal;

    node.customUi = get_customui();
    node.customUi.inputObjs.clear();
    for (auto& [name, paramObj] : m_inputObjs)
    {
        node.customUi.inputObjs.push_back(paramObj.exportParam());
    }
    if (m_nodecls == "SubOutput") {     //SubOutput节点tabs-groups-params为空，需单独导出primitiveInputs
        if (!node.customUi.inputPrims.empty() && !node.customUi.inputPrims[0].groups.empty()) {
            for (auto& [name, paramPrimitive] : m_inputPrims) {
                node.customUi.inputPrims[0].groups[0].params.push_back(paramPrimitive.exportParam());
            }
        }
    }
    else {
    for (auto &tab : node.customUi.inputPrims)
    {
        for (auto &group : tab.groups)
        {
            for (auto& param : group.params)
            {
                auto iter = m_inputPrims.find(param.name);
                if (iter != m_inputPrims.end())
                {
                    param = iter->second.exportParam();
                }
            }
        }
    }
    }

    node.customUi.outputPrims.clear();
    for (auto& [name, paramObj] : m_outputPrims)
    {
        node.customUi.outputPrims.push_back(paramObj.exportParam());
    }
    node.customUi.outputObjs.clear();
    for (auto& [name, paramObj] : m_outputObjs)
    {
        node.customUi.outputObjs.push_back(paramObj.exportParam());
    }
    return node;
}

bool NodeImpl::update_param(const std::string& param, zeno::reflect::Any new_value) {
    CORE_API_BATCH
    zeno::reflect::Any old_value;
    bool ret = update_param_impl(param, new_value, old_value);
    if (ret) {
        CALLBACK_NOTIFY(update_param, param, old_value, new_value)
        mark_dirty(true);
    }
    return ret;
}

bool NodeImpl::update_param_impl(const std::string& param, zeno::reflect::Any new_value, zeno::reflect::Any& old_value)
{
    auto& spParam = safe_at(m_inputPrims, param, "miss input param `" + param + "` on node `" + m_name + "`");
    bool isvalid = convertToEditVar(new_value, spParam.type);
    if (!isvalid) {
        zeno::log_error("cannot convert to edit variable");
        return false;
    }
    if (spParam.defl != new_value)
    {
        old_value = spParam.defl;
        spParam.defl = new_value;

        Graph* spGraph = m_pGraph;
        assert(spGraph);

        spGraph->onNodeParamUpdated(&spParam, old_value, new_value);
        initReferLinks(&spParam);
        checkParamsConstrain();
        return true;
    }
    return false;
}

bool NodeImpl::update_param_wildcard(const std::string& param, bool isWildcard)
{
    auto& spParam = safe_at(m_inputObjs, param, "miss input param `" + param + "` on node `" + m_name + "`");
    if (isWildcard != spParam.bWildcard) {
        spParam.bWildcard = isWildcard;
        CALLBACK_NOTIFY(update_param_wildcard, param, isWildcard)
        return true;
    }
    return false;
}

bool NodeImpl::update_param_socket_type(const std::string& param, SocketType type)
{
    CORE_API_BATCH
    auto& spParam = safe_at(m_inputObjs, param, "miss input param `" + param + "` on node `" + m_name + "`");
    if (type != spParam.socketType)
    {
        spParam.socketType = type;
#if 0
        if (type == Socket_Owning)
        {
            auto spGraph = graph;
            spGraph->removeLinks(m_name, true, param);
        }
#endif
        mark_dirty(true);
        CALLBACK_NOTIFY(update_param_socket_type, param, type)
        return true;
    }
    return false;
}

bool zeno::NodeImpl::update_param_type(const std::string& param, bool bPrim, bool bInput, ParamType type)
{
    CORE_API_BATCH
        if (bPrim)
        {
            auto& prims = bInput ? m_inputPrims : m_outputPrims;
            auto& prim = prims.find(param);
            if (prim != prims.end())
            {
                auto& spParam = prim->second;
                if (type != spParam.type)
                {
                    spParam.type = type;
                    CALLBACK_NOTIFY(update_param_type, param, type, bInput)

                    //默认值也要更新
                    if (bInput) {
                        zeno::reflect::Any defl = initAnyDeflValue(type);
                        convertToEditVar(defl, type);
                        update_param(spParam.name, defl);
                    }
                    return true;
                }
            }
        }
        else 
        {
            auto& objects = bInput ? m_inputObjs : m_outputObjs;
            auto& object = objects.find(param);
            if (object != objects.end())
            {
                auto& spParam = object->second;
                if (type != spParam.type)
                {
                    spParam.type = type;
                    CALLBACK_NOTIFY(update_param_type, param, type, bInput)
                    return true;
                }
            }
        }
    return false;
}

bool zeno::NodeImpl::update_param_control(const std::string& param, ParamControl control)
{
    CORE_API_BATCH
    auto& spParam = safe_at(m_inputPrims, param, "miss input param `" + param + "` on node `" + m_name + "`");
    if (control != spParam.control)
    {
        spParam.control = control;
        CALLBACK_NOTIFY(update_param_control, param, control)
        return true;
    }
    return false;
}

bool zeno::NodeImpl::update_param_control_prop(const std::string& param, zeno::reflect::Any props)
{
    CORE_API_BATCH
    auto& spParam = safe_at(m_inputPrims, param, "miss input param `" + param + "` on node `" + m_name + "`");
    spParam.ctrlProps = props;
        CALLBACK_NOTIFY(update_param_control_prop, param, props)
        return true;
}

bool NodeImpl::update_param_visible(const std::string& name, bool bOn, bool bInput) {
    if (bInput) {
        if (auto iter = m_inputObjs.find(name); iter != m_inputObjs.end()) {
            auto& paramObj = iter->second;
            if (paramObj.bVisible != bOn) {
                paramObj.bVisible = bOn;
                return true;
            }
        }
        else if (auto iter = m_inputPrims.find(name); iter != m_inputPrims.end()){
            auto& paramPrim = iter->second;
            if (paramPrim.bVisible != bOn) {
                paramPrim.bVisible = bOn;
                return true;
            }
        }
    }
    else {
        if (auto iter = m_outputObjs.find(name); iter != m_outputObjs.end()) {
            auto& paramObj = iter->second;
            if (paramObj.bVisible != bOn) {
                paramObj.bVisible = bOn;
                return true;
            }
        }
        else if (auto iter = m_outputPrims.find(name); iter != m_outputPrims.end()) {
            auto& paramPrim = iter->second;
            if (paramPrim.bVisible != bOn) {
                paramPrim.bVisible = bOn;
                return true;
            }
        }
    }
    return false;
}

void NodeImpl::checkParamsConstrain() {
    //ZfxContext
    auto& funcMgr = zeno::getSession().funcManager;
    ZfxContext ctx;
    ctx.spNode = this;
    //对于所有带有约束的输入参数，调整其可见和可用情况

    std::set<std::string> adjInputs, adjOutputs;

    bool bParamPropChanged = false;
    for (const auto& [name, param] : m_inputObjs) {
        if (!param.constrain.empty()) {
            ctx.code = param.constrain;
            ctx.param_constrain.constrain_param = name;
            ctx.param_constrain.bInput = true;
            ZfxExecute zfx(ctx.code, &ctx);
            zfx.execute();
            if (ctx.param_constrain.update_nodeparam_prop) {
                bParamPropChanged = true;
                adjInputs.insert(name);
            }
        }
    }
    for (const auto& [name, param] : m_inputPrims) {
        if (!param.constrain.empty()) {
            ctx.code = param.constrain;
            ctx.param_constrain.constrain_param = name;
            ctx.param_constrain.bInput = true;
            ZfxExecute zfx(ctx.code, &ctx);
            zfx.execute();
            if (ctx.param_constrain.update_nodeparam_prop) {
                bParamPropChanged = true;
                adjInputs.insert(name);
            }
        }
    }
    for (const auto& [name, param] : m_outputPrims) {
        if (!param.constrain.empty()) {
            ctx.code = param.constrain;
            ctx.param_constrain.constrain_param = name;
            ctx.param_constrain.bInput = false;
            ZfxExecute zfx(ctx.code, &ctx);
            zfx.execute();
            if (ctx.param_constrain.update_nodeparam_prop) {
                bParamPropChanged = true;
                adjOutputs.insert(name);
            }
        }
    }
    for (const auto& [name, param] : m_outputObjs) {
        if (!param.constrain.empty()) {
            ctx.code = param.constrain;
            ctx.param_constrain.constrain_param = name;
            ctx.param_constrain.bInput = false;
            ZfxExecute zfx(ctx.code, &ctx);
            zfx.execute();
            if (ctx.param_constrain.update_nodeparam_prop) {
                bParamPropChanged = true;
                adjOutputs.insert(name);
            }
        }
    }

    if (bParamPropChanged) {
        //通知上层UI去统一更新
        CALLBACK_NOTIFY(update_visable_enable, this, adjInputs, adjOutputs)
    }
}

bool NodeImpl::update_param_enable(const std::string& name, bool bOn, bool bInput) {
    if (bInput) {
        if (auto iter = m_inputObjs.find(name); iter != m_inputObjs.end()) {
            auto& paramObj = iter->second;
            if (paramObj.bEnable != bOn) {
                paramObj.bEnable = bOn;
                return true;
            }
        }
        else if (auto iter = m_inputPrims.find(name); iter != m_inputPrims.end()) {
            auto& paramPrim = iter->second;
            if (paramPrim.bEnable != bOn) {
                paramPrim.bEnable = bOn;
                return true;
            }
        }
        else {
            return false;
        }
    }
    else {
        if (auto iter = m_outputObjs.find(name); iter != m_outputObjs.end()) {
            auto& paramObj = iter->second;
            if (paramObj.bEnable != bOn) {
                paramObj.bEnable = bOn;
                return true;
            }
        }
        else if (auto iter = m_outputPrims.find(name); iter != m_outputPrims.end()) {
            auto& paramPrim = iter->second;
            if (paramPrim.bEnable != bOn) {
                paramPrim.bEnable = bOn;
                return true;
            }
        }
        else {
            return false;
        }
    }
    return false;
}

bool zeno::NodeImpl::update_param_socket_visible(const std::string& param, bool bVisible, bool bInput)
{
    CORE_API_BATCH
    if (bInput) {
        auto& spParam = safe_at(m_inputPrims, param, "miss input param `" + param + "` on node `" + m_name + "`");
        if (spParam.bSocketVisible != bVisible)
        {
            spParam.bSocketVisible = bVisible;
            CALLBACK_NOTIFY(update_param_socket_visible, param, bVisible, bInput)
            return true;
        }
    }
    else {
        auto& spParam = safe_at(m_outputPrims, param, "miss output param `" + param + "` on node `" + m_name + "`");
        if (spParam.bSocketVisible != bVisible)
        {
            spParam.bSocketVisible = bVisible;
            CALLBACK_NOTIFY(update_param_socket_visible, param, bVisible, bInput)
                return true;
        }
    }
    return false;
}

void NodeImpl::update_param_color(const std::string& name, std::string& clr)
{
    CORE_API_BATCH
    CALLBACK_NOTIFY(update_param_color, name, clr)
}

void NodeImpl::update_layout(params_change_info& changes)
{
    CALLBACK_NOTIFY(update_layout, changes);
}

bool NodeImpl::is_loaded() const {
    return m_pNode != nullptr;
}

void NodeImpl::update_load_info(bool bDisable) {
    if (bDisable) {
        m_pNode.reset();
    }
    else {
        //重新加载，需要拿到INode的定义
        auto iter = zeno::getSession().nodeClasses.find(m_nodecls);
        if (iter == zeno::getSession().nodeClasses.end()) {
            throw makeError<UnimplError>("cannot load the nodeclass definition");
        }
        const std::unique_ptr<INodeClass>& defcls = iter->second;
        m_pNode = defcls->new_coreinst();
    }
    CALLBACK_NOTIFY(update_load_info, bDisable);
}

params_change_info NodeImpl::update_editparams(const ParamsUpdateInfo& params, bool bSubnetInit)
{
    //TODO: 这里只有primitive参数类型的情况，还需要整合obj参数的情况。
    std::set<std::string> inputs_old, outputs_old, obj_inputs_old, obj_outputs_old;
    for (const auto& [param_name, _] : m_inputPrims) {
        inputs_old.insert(param_name);
    }
    for (const auto& [param_name, _] : m_outputPrims) {
        outputs_old.insert(param_name);
    }
    for (const auto& [param_name, _] : m_inputObjs) {
        obj_inputs_old.insert(param_name);
    }
    for (const auto& [param_name, _] : m_outputObjs) {
        obj_outputs_old.insert(param_name);
    }

    params_change_info changes;

    for (auto _pair : params) {
        if (const auto& pParam = std::get_if<ParamObject>(&_pair.param))
        {
            const ParamObject& param = *pParam;
            const std::string oldname = _pair.oldName;
            const std::string newname = param.name;

            auto& in_outputs = param.bInput ? m_inputObjs : m_outputObjs;
            auto& new_params = param.bInput ? changes.new_inputs : changes.new_outputs;
            auto& remove_params = param.bInput ? changes.remove_inputs : changes.remove_outputs;
            auto& rename_params = param.bInput ? changes.rename_inputs : changes.rename_outputs;

            if (oldname.empty()) {
                //new added name.
                if (in_outputs.find(newname) != in_outputs.end()) {
                    // the new name happen to have the same name with the old name, but they are not the same param.
                    in_outputs.erase(newname);
                    if (param.bInput)
                        obj_inputs_old.erase(newname);
                    else
                        obj_outputs_old.erase(newname);

                    remove_params.insert(newname);
                }

                ObjectParam sparam;
                sparam.name = newname;
                sparam.type = param.type;
                sparam.bWildcard = param.bWildcard;
                if (!param.bInput) {
                    sparam.socketType = Socket_Output;
                }
                else {
                    sparam.socketType = param.socketType;
                }
                sparam.m_wpNode = this;
                in_outputs[newname] = std::move(sparam);

                new_params.insert(newname);
            }
            else if (in_outputs.find(oldname) != in_outputs.end()) {
                if (oldname != newname) {
                    //exist name changed.
                    in_outputs[newname] = std::move(in_outputs[oldname]);
                    in_outputs.erase(oldname);

                    rename_params.insert({ oldname, newname });
                }
                else {
                    //name stays.
                }

                if (param.bInput)
                    obj_inputs_old.erase(oldname);
                else
                    obj_outputs_old.erase(oldname);

                auto& spParam = in_outputs[newname];
                spParam.type = param.type;
                spParam.name = newname;
                spParam.bWildcard = param.bWildcard;
                if (param.bInput)
                {
                    update_param_socket_type(spParam.name, param.socketType);
                }
            }
            else {
                throw makeError<KeyError>(oldname, "the name does not exist on the node");
            }
        }
        else if (const auto& pParam = std::get_if<ParamPrimitive>(&_pair.param))
        {
            const ParamPrimitive& param = *pParam;
            const std::string oldname = _pair.oldName;
            const std::string newname = param.name;

            auto& in_outputs = param.bInput ? m_inputPrims : m_outputPrims;
            auto& new_params = param.bInput ? changes.new_inputs : changes.new_outputs;
            auto& remove_params = param.bInput ? changes.remove_inputs : changes.remove_outputs;
            auto& rename_params = param.bInput ? changes.rename_inputs : changes.rename_outputs;

            if (oldname.empty()) {
                //new added name.
                if (in_outputs.find(newname) != in_outputs.end()) {
                    // the new name happen to have the same name with the old name, but they are not the same param.
                    in_outputs.erase(newname);
                    if (param.bInput)
                        inputs_old.erase(newname);
                    else
                        outputs_old.erase(newname);

                    remove_params.insert(newname);
                }

                PrimitiveParam sparam;
                sparam.defl = param.defl;
                if (param.type == sparam.type) {
                    sparam.defl = initAnyDeflValue(sparam.type);
                }
                convertToEditVar(sparam.defl, param.type);
                sparam.name = newname;
                sparam.type = param.type;
                sparam.control = param.control;
                sparam.ctrlProps = param.ctrlProps;
                sparam.socketType = Socket_Primitve;
                sparam.bWildcard = param.bWildcard;
                sparam.m_wpNode = this;
                sparam.bSocketVisible = param.bSocketVisible;
                in_outputs[newname] = std::move(sparam);

                new_params.insert(newname);
            }
            else if (in_outputs.find(oldname) != in_outputs.end()) {
                if (oldname != newname) {
                    //exist name changed.
                    in_outputs[newname] = std::move(in_outputs[oldname]);
                    in_outputs.erase(oldname);

                    rename_params.insert({ oldname, newname });
                }
                else {
                    //name stays.
                }

                if (param.bInput)
                    inputs_old.erase(oldname);
                else
                    outputs_old.erase(oldname);

                auto& spParam = in_outputs[newname];
                spParam.defl = param.defl;
                if (param.type != spParam.type) {
                    spParam.defl = initAnyDeflValue(spParam.type);
                }
                convertToEditVar(spParam.defl, spParam.type);

                spParam.name = newname;
                spParam.socketType = Socket_Primitve;
                spParam.bWildcard = param.bWildcard;
                if (param.bInput)
                {
                    if (param.type != spParam.type)
                    {
                        update_param_type(spParam.name, true, param.bInput, param.type);
                        //update_param_type(spParam->name, param.type);
                        //if (auto spNode = subgraph->getNode(oldname))
                        //    spNode->update_param_type("port", param.type);
                    }
                    update_param_control(spParam.name, param.control);
                    update_param_control_prop(spParam.name, param.ctrlProps);
                }
            }
            else {
                throw makeError<KeyError>(oldname, "the name does not exist on the node");
            }
        }
    }

    Graph* spGraph = m_pGraph;

    //the left names are the names of params which will be removed.
    for (auto rem_name : inputs_old) {
        if (spGraph)
            spGraph->removeLinks(get_name(), true, rem_name);
        m_inputPrims.erase(rem_name);
        changes.remove_inputs.insert(rem_name);
    }

    for (auto rem_name : outputs_old) {
        if (spGraph)
            spGraph->removeLinks(get_name(), false, rem_name);
        m_outputPrims.erase(rem_name);
        changes.remove_outputs.insert(rem_name);
    }

    for (auto rem_name : obj_inputs_old) {
        if (spGraph)
            spGraph->removeLinks(get_name(), true, rem_name);
        m_inputObjs.erase(rem_name);
        changes.remove_inputs.insert(rem_name);
    }

    for (auto rem_name : obj_outputs_old) {
        if (spGraph)
            spGraph->removeLinks(get_name(), false, rem_name);
        m_outputObjs.erase(rem_name);
        changes.remove_outputs.insert(rem_name);
    }
    changes.inputs.clear();
    changes.outputs.clear();
    for (const auto& [param, _] : params) {
        if (auto paramPrim = std::get_if<ParamPrimitive>(&param))
        {
            if (paramPrim->bInput)
                changes.inputs.push_back(paramPrim->name);
            else
                changes.outputs.push_back(paramPrim->name);
        }
        else if (auto paramPrim = std::get_if<ParamObject>(&param))
        {
            if (paramPrim->bInput)
                changes.inputs.push_back(paramPrim->name);
            else
                changes.outputs.push_back(paramPrim->name);
        }
    }
    return changes;
}

void NodeImpl::trigger_update_params(const std::string& param, bool changed, params_change_info changes)
{
    if (changed)
        update_layout(changes);
}

void NodeImpl::init(const NodeData& dat)
{
    //IO init
    if (!dat.name.empty())
        m_name = dat.name;

    if (m_name == "selfinc") {
        int j;
        j = -0;
    }

    m_pos = dat.uipos;
    m_bView = dat.bView;
    if (m_bView) {
        Graph* spGraph = m_pGraph;
        assert(spGraph);
        spGraph->viewNodeUpdated(m_name, m_bView);
    }
    if (SubnetNode* pSubnetNode = dynamic_cast<SubnetNode*>(this))
    {
        pSubnetNode->setCustomUi(dat.customUi);
    }
    initParams(dat);
    m_dirty = true;
}

void NodeImpl::initParams(const NodeData& dat)
{
    for (const ParamObject& paramObj : dat.customUi.inputObjs)
    {
        auto iter = m_inputObjs.find(paramObj.name);
        if (iter == m_inputObjs.end()) {
            add_input_obj_param(paramObj);
            continue;
        }
        auto& sparam = iter->second;

        //如果不是子图，不能读写socketType，一律以节点定义处为准。（TODO: 如果涉及到转为owning，甚至有些obj连线要去掉）
        if (dat.type == Node_SubgraphNode || dat.type == Node_AssetInstance) {
            sparam.socketType = paramObj.socketType;
        }
    }
    for (auto tab : dat.customUi.inputPrims)
    {
        for (auto group : tab.groups)
        {
            for (auto param : group.params)
            {
                auto iter = m_inputPrims.find(param.name);
                if (iter == m_inputPrims.end()) {
                    add_input_prim_param(param);
                    continue;
                }
                auto& sparam = iter->second;
                convertToEditVar(param.defl, param.type);
                sparam.defl = param.defl;
                convertToEditVar(sparam.defl, param.type);

                // 普通子图的控件及参数类型，是由定义处决定的，而非IO值。
                //sparam.control = param.control;
                //sparam.ctrlProps = param.ctrlProps;
                //sparam.type = param.type;
                sparam.bSocketVisible = param.bSocketVisible;

                //graph记录$F相关节点
                if (Graph* spGraph = m_pGraph)
                    spGraph->parseNodeParamDependency(&sparam, sparam.defl);
            }
        }
    }
    for (const ParamPrimitive& param : dat.customUi.outputPrims)
    {
        auto iter = m_outputPrims.find(param.name);
        if (iter == m_outputPrims.end()) {
            add_output_prim_param(param);
            continue;
        }
        auto& sparam = iter->second;
        sparam.bSocketVisible = param.bSocketVisible;
        //sparam.type = param.type;
    }
    for (const ParamObject& paramObj : dat.customUi.outputObjs)
    {
        add_output_obj_param(paramObj);
    }
}

ShaderData NodeImpl::get_input_shader(const std::string& param, zeno::reflect::Any defl) {
    //这个函数其实就是特供ShaderFinalize
    auto iter = m_inputPrims.find(param);
    if (iter == m_inputPrims.end()) {
        throw makeError<UnimplError>("not valid param");
    }
    ShaderData shader;

    if (!iter->second.result.has_value()) {
        bool bSucceed = false;
        shader.data = AnyToNumeric(defl, bSucceed);
        if (!bSucceed) {
            throw makeError<UnimplError>("cannot get NumericValue on defl value");
        }
        return shader;
    }

    const Any& result = iter->second.result;
    if (result.type().hash_code() == gParamType_Shader) {
        shader = any_cast<ShaderData>(result);
    }
    else {
        bool bSucceed = false;
        shader.data = AnyToNumeric(result, bSucceed);
        if (!bSucceed) {
            throw makeError<UnimplError>("cannot get NumericValue");
        }
    }

    ParamPath uuidpath = this->get_uuid_path() + "/" + param;
    shader.curr_param = uuidpath;
    return shader;
}

bool NodeImpl::has_input(std::string const &id) const {
    //这个has_input在旧的语义里，代表的是input obj，如果有一些边没有连上，那么有一些参数值仅有默认值，未必会设这个input的，
    //还有一种情况，就是对象值是否有输入引入
    //这种情况要看旧版本怎么处理。
    //对于新版本而言，对于数值型输入，没有连上边仅有默认值，就不算has_input，有点奇怪，因此这里直接判断参数是否存在。
    auto iter = m_inputObjs.find(id);
    if (iter != m_inputObjs.end()) {
        return !iter->second.links.empty();
    }
    else {
        return m_inputPrims.find(id) != m_inputPrims.end();
    }
}

zany NodeImpl::get_input(std::string const &id) const {
    auto iter = m_inputPrims.find(id);
    if (iter != m_inputPrims.end()) {
        auto& val = iter->second.result;
        if (!val.has_value()) {
            return nullptr;
        }
        const ParamType paramType = iter->second.type;
        switch (paramType) {
            case zeno::types::gParamType_Int:
            case zeno::types::gParamType_Float:
            case zeno::types::gParamType_Bool:
            case zeno::types::gParamType_Vec2f:
            case zeno::types::gParamType_Vec2i:
            case zeno::types::gParamType_Vec3f:
            case zeno::types::gParamType_Vec3i:
            case zeno::types::gParamType_Vec4f:
            case zeno::types::gParamType_Vec4i:
            case gParamType_AnyNumeric:
            {
                //依然有很多节点用了NumericObject，为了兼容，需要套一层NumericObject出去。
                std::shared_ptr<NumericObject> spNum = std::make_shared<NumericObject>();
                const auto& anyType = val.type();
                if (anyType == zeno::reflect::type_info<int>()) {
                    spNum->set<int>(zeno::reflect::any_cast<int>(val));
                }
                else if (anyType == zeno::reflect::type_info<bool>()) {
                    spNum->set<int>(zeno::reflect::any_cast<bool>(val));
                }
                else if (anyType == zeno::reflect::type_info<float>()) {
                    spNum->set<float>(zeno::reflect::any_cast<float>(val));
                }
                else if (anyType == zeno::reflect::type_info<vec2i>()) {
                    spNum->set<vec2i>(zeno::reflect::any_cast<vec2i>(val));
                }
                else if (anyType == zeno::reflect::type_info<vec3i>()) {
                    spNum->set<vec3i>(zeno::reflect::any_cast<vec3i>(val));
                }
                else if (anyType == zeno::reflect::type_info<vec4i>()) {
                    spNum->set<vec4i>(zeno::reflect::any_cast<vec4i>(val));
                }
                else if (anyType == zeno::reflect::type_info<vec2f>()) {
                    spNum->set<vec2f>(zeno::reflect::any_cast<vec2f>(val));
                }
                else if (anyType == zeno::reflect::type_info<vec3f>()) {
                    spNum->set<vec3f>(zeno::reflect::any_cast<vec3f>(val));
                }
                else if (anyType == zeno::reflect::type_info<vec4f>()) {
                    spNum->set<vec4f>(zeno::reflect::any_cast<vec4f>(val));
                }
                else if (paramType == gParamType_AnyNumeric && 
                    (anyType == zeno::reflect::type_info<const char*>() ||
                     anyType == zeno::reflect::type_info<zeno::String>() ||
                     anyType == zeno::reflect::type_info<std::string>())) {
                    std::string str = zeno::any_cast_to_string(val);
                    return std::make_shared<StringObject>(str);
                }
                else
                {
                    //throw makeError<TypeError>(typeid(T));
                    //error, throw expection.
                }
                return spNum;
            }
            case zeno::types::gParamType_Matrix3:
            {
                std::shared_ptr<zeno::MatrixObject> matrixObj = std::make_shared<MatrixObject>();
                matrixObj->m = zeno::reflect::any_cast<glm::mat3>(val);
                return matrixObj;
            }
            case zeno::types::gParamType_Matrix4:
            {
                std::shared_ptr<zeno::MatrixObject> matrixObj = std::make_shared<MatrixObject>();
                matrixObj->m = zeno::reflect::any_cast<glm::mat4>(val);
                return matrixObj;
            }
            case zeno::types::gParamType_String:
            {
                std::string str = zeno::any_cast_to_string(val);
                return std::make_shared<StringObject>(str);
            }
            case zeno::types::gParamType_Shader:
            {
                throw makeError<UnimplError>("ShaderObject has been deprecated, you can get it by get_param_result, cast it into the type `ShaderData`");
            }
            default:
                return nullptr;
        }
    }
    else {
        auto iter2 = m_inputObjs.find(id);
        if (iter2 != m_inputObjs.end()) {
            return iter2->second.spObject;
        }
        throw makeError<KeyError>(id, "get_input");
    }
    return nullptr;
}

void NodeImpl::set_pos(std::pair<float, float> pos) {
    m_pos = pos;
    CALLBACK_NOTIFY(set_pos, m_pos)
}

std::pair<float, float> NodeImpl::get_pos() const {
    return m_pos;
}

bool NodeImpl::in_asset_file() const {
    Graph* spGraph = m_pGraph;
    assert(spGraph);
    return getSession().assets->isAssetGraph(spGraph);
}

bool NodeImpl::set_primitive_input(std::string const& id, const zeno::reflect::Any& val) {
    auto iter = m_inputPrims.find(id);
    if (iter == m_inputPrims.end())
        return false;
    iter->second.result = val;
}

bool NodeImpl::set_primitive_output(std::string const& id, const zeno::reflect::Any& val) {
    auto iter = m_outputPrims.find(id);
    assert(iter != m_outputPrims.end());
    if (iter == m_outputPrims.end())
        return false;
    iter->second.result = val;
}

bool NodeImpl::set_output(std::string const& param, zany obj) {
    //只给旧节点模块使用，如果函数暴露reflect::Any，就会迫使所有使用这个函数的cpp文件include headers
    //会增加程序体积以及编译时间，待后续生成文件优化后再考虑处理。
    auto iter = m_outputObjs.find(param);
    if (iter != m_outputObjs.end()) {
            iter->second.spObject = obj;
        return true;
    }
    else {
        auto iter2 = m_outputPrims.find(param);
        if (iter2 != m_outputPrims.end()) {
            //兼容以前NumericObject的情况
            if (auto numObject = std::dynamic_pointer_cast<NumericObject>(obj)) {
                const auto& val = numObject->value;
                if (std::holds_alternative<int>(val))
                {
                    iter2->second.result = std::get<int>(val);
                }
                else if (std::holds_alternative<float>(val))
                {
                    iter2->second.result = std::get<float>(val);
                }
                else if (std::holds_alternative<vec2i>(val))
                {
                    iter2->second.result = std::get<vec2i>(val);
                }
                else if (std::holds_alternative<vec2f>(val))
                {
                    iter2->second.result = std::get<vec2f>(val);
                }
                else if (std::holds_alternative<vec3i>(val))
                {
                    iter2->second.result = std::get<vec3i>(val);
                }
                else if (std::holds_alternative<vec3f>(val))
                {
                    iter2->second.result = std::get<vec3f>(val);
                }
                else if (std::holds_alternative<vec4i>(val))
                {
                    iter2->second.result = std::get<vec4i>(val);
                }
                else if (std::holds_alternative<vec4f>(val))
                {
                    iter2->second.result = std::get<vec4f>(val);
                }
                else
                {
                    //throw makeError<TypeError>(typeid(T));
                    //error, throw expection.
                }
            }
            else if (auto strObject = std::dynamic_pointer_cast<StringObject>(obj)) {
                const auto& val = strObject->value;
                iter2->second.result = val;
            }
            return true;
        }
    }
    return false;
}

zany NodeImpl::get_default_output_object() {
    if (m_nodecls == "SubOutput") {
        return get_input("port");
    }
    if (m_outputObjs.empty())
        return nullptr;
    return m_outputObjs.begin()->second.spObject;
}

zany NodeImpl::get_output_obj(std::string const& param) {
    auto& spParam = safe_at(m_outputObjs, param, "miss output param `" + param + "` on node `" + m_name + "`");
    return spParam.spObject;
}

std::vector<zany> NodeImpl::get_output_objs() {
    std::vector<zany> objs;
    for (const auto& [name, objparam] : m_outputObjs) {
        objs.push_back(objparam.spObject);
    }
    return objs;
}

TempNodeCaller NodeImpl::temp_node(std::string const &id) {
    //TODO: deprecated
    Graph* spGraph = m_pGraph;
    assert(spGraph);
    return TempNodeCaller(spGraph, id);
}

float NodeImpl::resolve(const std::string& expression, const ParamType type)
{
    std::string code = expression;
    Formula fmla(code, get_path());
    int ret = fmla.parse();
    if (ret == 0)
    {
        auto& funcMgr = zeno::getSession().funcManager;
        auto& astRoot = fmla.getASTResult();
        ZfxContext ctx;
        ctx.code = code;
        ctx.spNode = this;
        zfxvariant res = funcMgr->calc(astRoot, &ctx);
        //是否需要整合calc和executezfx?
        //funcMgr->executeZfx(astRoot, &ctx);

        if (std::holds_alternative<int>(res)) {
            return std::get<int>(res);
        }
        else if (std::holds_alternative<float>(res)) {
            return std::get<float>(res);
        }
        else {
            throw makeError<UnimplError>();
        }
    }
    else {
        //TODO: kframe issues
    }
}

void NodeImpl::initTypeBase(zeno::reflect::TypeBase* pTypeBase)
{
    m_pTypebase = pTypeBase;
}

bool NodeImpl::is_continue_to_run() {
    return false;
}

void NodeImpl::increment() {

}

void NodeImpl::reset_forloop_settings() {

}

std::shared_ptr<IObject> NodeImpl::get_iterate_object() {
    return nullptr;
}

std::vector<std::pair<std::string, bool>> zeno::NodeImpl::getWildCardParams(const std::string& param_name, bool bPrim)
{
    std::vector<std::pair<std::string, bool>> params;
    if (bPrim)
    {
        std::string wildCardGroup;
        if (m_inputPrims.find(param_name) != m_inputPrims.end())
        {
            wildCardGroup = m_inputPrims.find(param_name)->second.wildCardGroup;
        }
        else if (m_outputPrims.find(param_name) != m_outputPrims.end())
        {
            wildCardGroup = m_outputPrims.find(param_name)->second.wildCardGroup;
        }
        for (const auto&[name, spParam] : m_inputPrims)
        {
            if (spParam.wildCardGroup == wildCardGroup)
            {
                if (!wildCardGroup.empty() || param_name == name) {
                    params.push_back({name, true});
            }
        }
        }
        for (const auto& [name, spParam] : m_outputPrims)
        {
            if (spParam.wildCardGroup == wildCardGroup)
            {
                if (!wildCardGroup.empty() || param_name == name) {
                    params.push_back({name, false});
            }
        }
        }
    } 
    else
    {
        std::string wildCardGroup;
        if (m_inputObjs.find(param_name) != m_inputObjs.end())
        {
            wildCardGroup = m_inputObjs.find(param_name)->second.wildCardGroup;
        }
        else if (m_outputObjs.find(param_name) != m_outputObjs.end())
        {
            wildCardGroup = m_outputObjs.find(param_name)->second.wildCardGroup;
        }
        for (const auto& [name, spParam] : m_inputObjs)
        {
            if (spParam.wildCardGroup == wildCardGroup)
            {
                if (!wildCardGroup.empty() || param_name == name) {
                    params.push_back({name, true});
            }
        }
        }
        for (const auto& [name, spParam] : m_outputObjs)
        {
            if (spParam.wildCardGroup == wildCardGroup)
            {
                if (!wildCardGroup.empty() || param_name == name) {
                    params.push_back({name, false});
            }
        }
    }
    }
    return params;
}

void zeno::NodeImpl::getParamTypeAndSocketType(const std::string& param_name, bool bPrim, bool bInput, ParamType& paramType, SocketType& socketType, bool& bWildcard)
{
    if (bPrim) {
        auto iter = bInput ? m_inputPrims.find(param_name) : m_outputPrims.find(param_name);
        if (bInput ? (iter != m_inputPrims.end()) : (iter != m_outputPrims.end())) {
            paramType = iter->second.type;
            socketType = iter->second.socketType;
            bWildcard = iter->second.bWildcard;
            return;
    }
    }
    else {
        auto iter = bInput ? m_inputObjs.find(param_name) : m_outputObjs.find(param_name);
        if (bInput ? (iter != m_inputObjs.end()) : (iter != m_outputObjs.end())) {
            paramType = iter->second.type;
            socketType = iter->second.socketType;
            bWildcard = iter->second.bWildcard;
            return;
        }
    }
    paramType = Param_Null;
    socketType = Socket_Primitve;
}

template<class T, class E> T NodeImpl::resolveVec(const zeno::reflect::Any& defl, const ParamType type)
{
    if (zeno::reflect::get_type<E>() == defl.type()) {
        E vec = zeno::reflect::any_cast<E>(defl);
        T vecnum;
        for (int i = 0; i < vec.size(); i++) {
            float fVal = resolve(vec[i], type);
            vecnum[i] = fVal;
        }
        return vecnum;
    }
    else if (zeno::reflect::get_type<T>() == defl.type()) {
        return zeno::reflect::any_cast<T>(defl);
    }
    else {
        throw;
    }
}

}

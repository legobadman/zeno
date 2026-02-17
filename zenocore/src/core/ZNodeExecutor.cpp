#include <zeno/core/ZNodeExecutor.h>
#include <zeno/core/Graph.h>
#include <zeno/core/Descriptor.h>
#include <zeno/core/Session.h>
#include <zeno/core/Assets.h>
#include <zeno/core/INodeClass.h>
#include <zeno/types/DummyObject.h>
#include <zeno/extra/GlobalState.h>
#include <zeno/extra/DirtyChecker.h>
#include <zeno/extra/foreach.h>
#include <zeno/utils/Error.h>
#include <zeno/utils/string.h>
#include <zeno/funcs/ParseObjectFromUi.h>
#ifdef ZENO_BENCHMARKING
#include <zeno/utils/Timer.h>
#endif
#include <iobject2.h>
#include <zeno/utils/safe_at.h>
#include <zeno/utils/logger.h>
#include <zeno/utils/uuid.h>
#include <zeno/core/CoreParam.h>
#include <zeno/ListObject.h>
#include <zeno/utils/helper.h>
#include <zeno/extra/GraphException.h>
#include <zeno/formula/formula.h>
#include <zeno/core/FunctionManager.h>
#include <reflect/type.hpp>
#include <reflect/container/arraylist>
#include <reflect/metadata.hpp>
#include <zeno/types/ListObject.h>
#include <zeno/types/ListObject_impl.h>
#include <zeno/types/MeshObject.h>
#include "zeno_types/reflect/reflection.generated.hpp"
#include <zeno/core/reflectdef.h>
#include <zeno/formula/zfxexecute.h>
#include <zeno/extra/CalcContext.h>
#include <filesystem>
#include <zeno/utils/interfaceutil.h>
#include <zeno/core/ZNodeParams.h>
#include <zeno/core/ZNodeStatus.h>
//#include <Python.h>
//#include <pybind11/pybind11.h>


using namespace zeno::reflect;
using namespace zeno::types;


namespace zeno {

    static std::string get_path(ZNode* pNode) {
        return pNode->getNodeStatus().get_path();
    }

    static std::string get_uuid_path(ZNode* pNode) {
        return pNode->getNodeStatus().get_uuid_path();
    }

    static std::string get_uuid(ZNode* pNode) {
        return pNode->getNodeStatus().get_uuid();
    }

    static int get_frame_id() {
        return getSession().globalState->getFrameId();
    }

    static std::string get_name(ZNode* pNode) {
        return pNode->getNodeStatus().get_name();
    }

    static std::string ws2s(std::wstring const& wstr) {
        std::setlocale(LC_ALL, "");  // 设置为系统默认 locale
        size_t len = std::wcstombs(nullptr, wstr.c_str(), 0);
        std::string str(len, '\0');
        std::wcstombs(&str[0], wstr.c_str(), len);
        return str;
    }

    static std::string replaceTokens(const std::string& input,
        const std::unordered_map<std::string, std::string>& replacements)
    {
        std::string result = input;
        for (const auto& [key, value] : replacements) {
            size_t pos = 0;
            while ((pos = result.find(key, pos)) != std::string::npos) {
                result.replace(pos, key.length(), value);
                pos += value.length(); // 跳过已替换的部分
            }
        }
        return result;
    }


    ZNodeExecutor::ZNodeExecutor(ZNode* pNodeRepo, INode2* pNode, void (*dtor)(INode2*))
        : m_upNode2(pNode, dtor)
        , m_pNodeRepo(pNodeRepo)
    {
    }

    void ZNodeExecutor::initAfterIO() {
        m_dirty = true;
        if (m_pNodeRepo->getNodeStatus().get_nodecls() == "FrameCache") {
            //有可能上一次已经缓存了内容，如果我们允许上一次的缓存留下来的话，就可以标为FrameChanged
            //那就意味着，如果想清理缓存，只能让用户手动清理，因为机制不好理解什么时候缓存失效
            m_dirtyReason = Dirty_FrameChanged;
        }
        else {
            m_dirtyReason = Dirty_All;
        }
    }

    void ZNodeExecutor::execute(CalcContext* pContext)
    {
        std::lock_guard scope(m_mutex);
        doApply(pContext);
    }

    zany2 ZNodeExecutor::execute_get_object(const ExecuteContext& exec_context)
    {
        //锁的粒度可以更精细化，比如没有apply和takeover，在只读的情况下，可以释放锁
        //另一方面，如果多个节点想获取同一个节点的输出，就得排队了
        std::lock_guard scope(m_mutex);

        doApply(exec_context.pContext);

        zany2 res;
        //TODO: take over要设计成只限制一对一连接，否则遇到没法clone的对象，就没法move给两个下游了
        if (m_pNodeRepo->getNodeStatus().is_nocache()) {
            //TODO: 是否要在这里清理outputNode?
            //其实可以，因为这里并不是存粹的数学函数，必然有修改当前节点状态的可能。
            res = m_pNodeRepo->getNodeParams().move_output(exec_context.out_param);
            mark_takeover();
        }
        else {
            auto result = m_pNodeRepo->getNodeParams().get_output_obj(exec_context.out_param);
            if (!result) {
                throw makeNodeError<UnimplError>(get_path(m_pNodeRepo), "no result");
            }
            res = zany2(result->clone());
        }
        return res;

    }

    zeno::reflect::Any ZNodeExecutor::execute_get_numeric(const ExecuteContext& exec_context)
    {
        return m_pNodeRepo->getNodeParams().get_param_result(exec_context.out_param);
    }

    void ZNodeExecutor::doOnlyApply()
    {
        apply();
    }

    // ============================================================
    // dirty / clean
    // ============================================================

    void ZNodeExecutor::mark_dirty(
        bool bOn,
        DirtyReason reason,
        bool bWholeSubnet,
        bool bRecursively)
    {
        if (bOn == (m_status != Node_Clean))
            return;

        m_dirtyReason = reason;
        dirty_changed(bOn, reason, bWholeSubnet, bRecursively);
    }

    void ZNodeExecutor::mark_clean()
    {
        m_status = Node_Clean;
    }

    void ZNodeExecutor::dirty_changed(
        bool bOn,
        DirtyReason reason,
        bool bWholeSubnet,
        bool bRecursively)
    {
#if AI_CODE_CORRECT
        auto params = m_pNodeRepo->getNodeParams();

        if (!bOn) return;

        m_status = Node_DirtyReadyToRun;

        // 清理自身输出缓存
        clearCalcResults();

        if (!bRecursively) return;

        // 向下游传播 dirty（Primitive）
        for (const auto& [_, p] : params->get_output_prim_params2()) {
            for (auto& link : p.links) {
                if (!link) continue;
                auto* dstNode = link->edge.dstNode;
                if (!dstNode) continue;
                dstNode->executor()->mark_dirty(true, reason, bWholeSubnet, true);
            }
        }

        // 向下游传播 dirty（Object）
        for (auto& [_, o] : params->get_output_object_params2()) {
            for (auto& link : o.links) {
                if (!link) continue;
                auto* dstNode = link->edge.dstNode;
                if (!dstNode) continue;
                dstNode->executor()->mark_dirty(true, reason, bWholeSubnet, true);
            }
        }
#endif
    }

    void ZNodeExecutor::clearCalcResults()
    {
        auto& params = m_pNodeRepo->getNodeParams();
        for (auto& [_, p] : params.get_output_prim_params2()) {
            p.result.reset();
        }
        for (auto& [_, o] : params.get_output_object_params2()) {
            o.spObject.reset();
        }
    }

    float ZNodeExecutor::time() const
    {
        return 0.f;
    }

    // ============================================================
    // foreach / loop
    // ============================================================

    bool ZNodeExecutor::is_continue_to_run()
    {
        return false;
    }

    void ZNodeExecutor::increment()
    {
    }

    void ZNodeExecutor::reset_forloop_settings()
    {
    }

    std::shared_ptr<IObject2> ZNodeExecutor::get_iterate_object()
    {
        return nullptr;
    }

    // ============================================================
    // state helpers
    // ============================================================

    void ZNodeExecutor::onInterrupted()
    {
        mark_dirty(true);
        mark_previous_ref_dirty();
    }

    void ZNodeExecutor::mark_previous_ref_dirty()
    {
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

    void ZNodeExecutor::update_out_objs_key()
    {
        for (auto const& [name, param] : m_pNodeRepo->getNodeParams().get_output_object_params2())
        {
            if (param.spObject)
            {
                //目前节点处所看到的object，都隶属于此节点本身。
                if (get_object_key(param.spObject).empty()) {
                    param.spObject->update_key(stdString2zs(m_pNodeRepo->get_uuid()));
                }
            }
        }
    }

    void ZNodeExecutor::mark_dirty_objs()
    {
        for (auto const& [name, param] : m_pNodeRepo->getNodeParams().get_output_object_params2())
        {
            if (param.spObject) {
                assert(param.spObject);
                if (get_object_key(param.spObject.get()).empty()) {
                    continue;
                }
            }
        }
    }

    void ZNodeExecutor::reportStatus(bool bDirty, NodeRunStatus status)
    {
        m_status = status;
        m_dirty = bDirty;
        auto& sess = zeno::getSession();
        if (status == Node_RunSucceed) {
            sess.globalState->update_consume_time(this->time());
        }
        sess.reportNodeStatus(get_uuid_path(m_pNodeRepo), bDirty, status);
    }

    void ZNodeExecutor::mark_takeover()
    {
        m_takenover = true;
        clearCalcResults();
        mark_dirty(false);
        reportStatus(false, Node_ResultTaken);
    }

    bool ZNodeExecutor::is_takenover() const
    {
        return m_takenover;
    }

    bool ZNodeExecutor::is_dirty() const {
        return m_dirty;
    }

    bool ZNodeExecutor::is_upstream_dirty(const std::string& in_param) const {
        auto& _inputObjs = m_pNodeRepo->getNodeParams().get_input_object_params2();
        auto& _inputPrims = m_pNodeRepo->getNodeParams().get_input_prim_params2();

        if (auto iter = _inputObjs.find(in_param); iter != _inputObjs.end()) {
            const auto& param = iter->second;
            for (auto spLink : param.links) {
                auto outNode = spLink->fromparam->m_wpNode;
                NodeRunStatus status = outNode->getNodeExecutor().get_run_status();
                if (status != Node_Clean) {
                    return true;
                }
            }
            return false;
        }
        else if (auto iter = _inputPrims.find(in_param); iter != _inputPrims.end()) {
            const auto& param = iter->second;
            for (auto spLink : param.links) {
                auto outNode = spLink->fromparam->m_wpNode;
                NodeRunStatus status = outNode->getNodeExecutor().get_run_status();
                if (status != Node_Clean) {
                    return true;
                }
            }
            return false;
        }
        else {
            throw makeNodeError<UnimplError>(m_pNodeRepo->getNodeStatus().get_path(),
                "the param is not exist");
        }
    }

    void ZNodeExecutor::check_break_and_return()
    {
        if (zeno::getSession().is_interrupted()) {
            m_status = Node_DirtyReadyToRun;
            reportStatus(m_dirty, m_status);
            throw makeNodeError<InterruputError>(get_path(m_pNodeRepo), get_path(m_pNodeRepo));
        }
    }

    void ZNodeExecutor::complete()
    {
    }

    INode2* ZNodeExecutor::coreNode() const {
        return m_upNode2.get();
    }

    void ZNodeExecutor::apply()
    {
        if (m_upNode2) {
            try {
                if (m_upNode2) {
                    //这里属于core层面，可以抛异常，但节点不能抛，否则破坏二进制兼容
                    ZErrorCode err = m_upNode2->apply(&m_pNodeRepo->getNodeParams());
                    if (err != ZErr_OK) {
                        auto err = getSession().globalState->get_report_error();
                        //TODO: 细化各种错误
                        throw makeNodeError<UnimplError>(get_path(m_pNodeRepo), err);
                    }
                }
            }
            catch (ErrorException const& e) {
                if (e.get_node_info().empty()) {
                    throw ErrorException(get_path(m_pNodeRepo), e.getError());
                }
                else {
                    throw e;
                }
            }
            catch (std::exception const& e) {
                std::string err = e.what();
                throw makeNodeError<StdError>(get_path(m_pNodeRepo), std::current_exception());
            }
            catch (...) {
                throw makeNodeError<UnimplError>(get_path(m_pNodeRepo), "unknown error");
            }
        }
        else {
            throw makeNodeError<UnimplError>(get_path(m_pNodeRepo), "the node has been uninstalled");
        }
    }

    void ZNodeExecutor::doApply(CalcContext* pContext)
    {
        if (!m_dirty)
            return;

        check_break_and_return();

        assert(pContext);
        std::string uuid_path = get_uuid_path(m_pNodeRepo);

        scope_exit spUuidRecord([=] {
            std::lock_guard scope(pContext->mtx);
            pContext->visited_nodes.erase(uuid_path);
            });

        {
            std::lock_guard scope(pContext->mtx);
            if (pContext->visited_nodes.find(uuid_path) != pContext->visited_nodes.end()) {
                throw makeNodeError<UnimplError>(get_path(m_pNodeRepo), "cycle reference occurs!");
            }
            pContext->visited_nodes.insert(uuid_path);
        }

        const auto& node_name = m_pNodeRepo->getNodeStatus().get_name();
        const auto& node_cls = m_pNodeRepo->getNodeStatus().get_nodecls();
#if 1
        //just for debug the version of relwithdbinfo
        if (m_bypass) {
            m_pNodeRepo->getNodeStatus().set_name(node_name);
        }
        if (node_name == "ForEachEnd6") {//}&& pContext->curr_iter == 1) {
            m_pNodeRepo->getNodeStatus().set_name(node_name);
        }
#endif

        const auto& _outputobjs = m_pNodeRepo->getNodeParams().get_output_object_params2();
        for (auto const& [name, param] : _outputobjs) {
            if (param.type == gParamType_List && param.spObject) {
                auto list = static_cast<ListObject*>(param.spObject.get());
                list->m_modify.clear();
                list->m_new_added.clear();
                list->m_new_removed.clear();
            }
        }

        if (node_cls == "TimeShift") {
            preApplyTimeshift(pContext);
        }
        else if (node_cls == "SwitchIf") {
            preApply_SwitchIf(pContext);
        }
        else if (node_cls == "SwitchBetween") {
            preApply_SwitchBetween(pContext);
        }
        else if (node_cls == "ForEachEnd") {
            preApply_Primitives(pContext);
        }
        else if (node_cls == "FrameCache") {
            preApply_FrameCache(pContext);
        }
        else {
            preApply(pContext);
        }

        check_break_and_return();

        log_debug("==> enter {}", node_name);
        {
#ifdef ZENO_BENCHMARKING
            //Timer _(get_name(m_pNodeRepo));
#endif
        //暂时废弃bypass，先作为一个debug节点
            if (false && m_bypass) {
                bypass();
            }
            else {
                reportStatus(true, Node_Running);
                if (node_cls == "ForEachEnd") {
                    foreachend_apply(pContext);
                }
                else {
                    apply();
                }
            }
        }
        log_debug("==> leave {}", node_name);

        update_out_objs_key();
        reportStatus(false, Node_RunSucceed);
    }

    void ZNodeExecutor::doApply_Parameter(std::string const& name, CalcContext* pContext)
    {
        if (!m_dirty) {
            return;
        }

        std::string uuid_path = get_uuid_path(m_pNodeRepo) + "/" + name;
        if (pContext->uuid_node_params.find(uuid_path) != pContext->uuid_node_params.end()) {
            throw makeNodeError<UnimplError>(get_path(m_pNodeRepo), "cycle reference occurs when refer paramters!");
        }

        scope_exit scope_apply_param([&]() { pContext->uuid_node_params.erase(uuid_path); });
        pContext->uuid_node_params.insert(uuid_path);

        requireInput(name, pContext);
    }

    // ============================================================
    // param processing
    // ============================================================

    zeno::reflect::Any ZNodeExecutor::processPrimitive(PrimitiveParam* in_param)
    {
        //没有连线的情况，这时候直接取defl作为result。
        if (!in_param) {
            return Any();
        }

        ;

        int frame = getSession().globalState->getFrameId();

        const std::string& name = in_param->name;
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
        case gParamType_AnyNumeric: //暂时不考虑默认值是向量的情况
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
                        int frame = get_frame_id();
                        return arg.eval(frame);
                    }
                    else {
                        throw makeNodeError<UnimplError>(get_path(m_pNodeRepo), "error type with `gParamType_PrimVariant`");
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
            if (in_param->control != Lineedit &&
                in_param->control != Multiline &&
                in_param->control != ReadPathEdit &&
                in_param->control != WritePathEdit &&
                in_param->control != NullControl/*初始化的时候只有type没有control，而ui会自动初始化*/) {
                //不可能是公式
                break;
            }
            if (in_param->control == CodeEditor) {
                //单独在wrangle里parse execute.
                break;
            }

            //有很多ref子图中字符串类型的参数，所以一切字符串都要parse
            const std::string& code = any_cast_to_string(defl);
            if (!code.empty()) {
                result = resolve_string(code, code);
            }
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
                        int frame = get_frame_id();
                        return arg.eval(frame);
                    }
                    else {
                        throw makeNodeError<UnimplError>(get_path(m_pNodeRepo));
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
        case gParamType_ListOfMat4:
        {
            if (in_param->links.size() == 1 && in_param->links.front()->toparam->type == gParamType_ListOfMat4) {
                //TODO
            }
            else {
                for (auto link : in_param->links) {

                }
            }
            break;
        }
        case gParamType_Shader:
        {
            throw makeNodeError<UnimplError>(get_path(m_pNodeRepo), "no defl value supported on the param with type `gParamType_Shader`, please connect the link by outside");
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
                        throw makeNodeError<UnimplError>(get_path(m_pNodeRepo));
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
        case gParamType_Curve:
        {
            result = defl;
            break;
        }
        }
        return result;
    }

    std::unique_ptr<ListObject> ZNodeExecutor::processList(ObjectParam* in_param, CalcContext* pContext)
    {
        assert(gParamType_List == in_param->type);

        bool bDirectLink = false;

        const auto& nodecls = m_pNodeRepo->getNodeStatus().get_nodecls();
        if (nodecls == "FormSceneTree") {
            int j;
            j = 0;
        }

        if (in_param->links.size() == 1)
        {
            std::shared_ptr<ObjectLink> spLink = in_param->links.front();
            auto out_param = spLink->fromparam;
            auto outNode = out_param->m_wpNode;

            auto list_register_all_items = [&](ListObject* listobj) {
                if (!listobj->has_change_info()) {
                    //上游没有修改信息，只能全部加进来，还要比较有哪些被删掉
                    for (const auto& obj : listobj->m_objects) {
                        std::string key = get_object_key(obj);
                        if (key.empty()) throw makeNodeError<UnimplError>(get_path(m_pNodeRepo), "there is object in list with empty key");
                        //直接全部收集
                        listobj->m_new_added.insert(key);
                    }
                }
            };

            if (outNode->getNodeExecutor().is_takenover()) {
                //可能上一次计算被taken了
                return nullptr;
            }

            if ((out_param->type == in_param->type || out_param->type == gParamType_IObject2) &&
                spLink->tokey.empty())
            {
                std::unique_ptr<ListObject> spList;
                bool bAllTaken = false;     //输出参数所有的链路（包括本链路）都被获取了
                if (spLink->upstream_task.valid()) {
                    auto outResult = spLink->upstream_task.get();   //outResult已经是本节点输入参数所有，不属于outnode了
                    auto _spList = dynamic_cast<ListObject*>(outResult.get());
                    if (!_spList) {
                        throw makeNodeError(get_path(m_pNodeRepo), "no list object received");
                    }
                    spList.reset(static_cast<ListObject*>(outResult.release()));
                    //无论原来有没有缓存，上游的list已经脏了，就干脆直接换新的，不去一个个比较了
                    list_register_all_items(spList.get());
                    spList->update_key(get_object_key(out_param->spObject).c_str());
                }
                else {
                    assert(!outNode->getNodeExecutor().is_dirty());
                    //上游已经算好了，但当前的输入没有建立缓存，就得从上游拷贝一下
                    if (in_param->spObject) {
                        //相当于转一次又给回外面。。。
                        spList = safe_uniqueptr_cast<ListObject>(std::move(in_param->spObject));
                    }
                    else {
                        //outNode已经算好了，直接拿应该不会导致race condition
                        spList = safe_uniqueptr_cast<ListObject>(zany2(out_param->spObject->clone()));
                        //新的list，这里全部内容都要登记到new_added.
                        list_register_all_items(spList.get());
                        spList->update_key(get_object_key(out_param->spObject).c_str());
                    }
                }
                if (!spList) {
                    throw makeNodeError<UnimplError>(get_path(m_pNodeRepo), "no outResult List from output");
                }
                bDirectLink = true;
                add_prefix_key(spList.get(), m_pNodeRepo->get_uuid());
                return spList;
            }
        }
        if (!bDirectLink)
        {
            auto spList = create_ListObject();
            auto cachedList = static_cast<ListObject*>(in_param->spObject.get());
            std::set<std::string> old_list;
            if (cachedList) {
                for (const auto& obj : cachedList->m_objects) {
                    old_list.insert(get_object_key(obj));
                }
            }

            //TODO: 多个物体作为List的元素，如果其中有一个被take over，要怎么处理？
            for (const auto& spLink : in_param->links)
            {
                auto out_param = spLink->fromparam;
                std::string upstream_obj_key;
                std::string new_obj_key;
                auto outNode = out_param->m_wpNode;
                if (outNode->getNodeExecutor().is_takenover()) {
                    continue;
                }
                if (spLink->upstream_task.valid()) {
                    //有任务发起，说明上游新增了或者修改了某一个节点
                    auto outResult = spLink->upstream_task.get();
                    upstream_obj_key = get_object_key(out_param->spObject);
                    outResult->update_key(stdString2zs(upstream_obj_key));
                    add_prefix_key(outResult.get(), m_pNodeRepo->get_uuid());
                    new_obj_key = get_object_key(outResult);

                    spList->push_back2(std::move(outResult));

                    if (old_list.find(new_obj_key) != old_list.end()) {
                        //旧的列表有这一项，说明是修改的
                        spList->m_modify.insert(new_obj_key);
                    }
                    else {
                        //旧的列表没有，现在发现新的
                        spList->m_new_added.insert(new_obj_key);
                    }
                }
                else {
                    //没有任务发起，但上游可能已经有缓存好的结果，直接加到list即可
                    assert(!outNode->getNodeExecutor().is_dirty());
                    //想知道是不是新增，要与oldList对比
                    upstream_obj_key = get_object_key(out_param->spObject);
                    new_obj_key = m_pNodeRepo->get_uuid() + '\\' + upstream_obj_key;
                    if (old_list.find(new_obj_key) != old_list.end()) {
                        //直接从缓存取就行
                        for (const auto& obj : cachedList->m_objects) {
                            if (get_object_key(obj) == new_obj_key) {
                                spList->push_back(obj->clone());
                                break;
                            }
                        }
                    }
                    else {
                        //上游节点不脏，但边新增到list，这时候需要标脏这个obj，否则渲染端没法认出
                        auto new_obj = zany2(out_param->spObject->clone());
                        new_obj->update_key(stdString2zs(upstream_obj_key));
                        add_prefix_key(new_obj.get(), m_pNodeRepo->get_uuid());
                        spList->m_modify.insert(get_object_key(new_obj));

                        spList->push_back2(std::move(new_obj));
                    }
                }
                old_list.erase(new_obj_key);
            }
            //从old_list剩下的，就是要被删除的元素
            spList->m_new_removed = old_list;
            spList->update_key(stdString2zs(m_pNodeRepo->get_uuid()));
            return spList;
        }
        return nullptr;
    }

    bool ZNodeExecutor::receiveOutputObj(
        ObjectParam* in_param,
        ZNode* outNode,
        ObjectParam* out_param)
    {
        //在此版本里，只有克隆，每个对象只有一个节点关联，虽然激进，但可以充分测试属性数据共享在面对
        //内存暴涨时的冲击，能优化到什么程度

        bool bCloned = true;
        //如果outputobj是 PrimitiveObject，则不走拷贝，而是move，如果List和Dict含有哪怕只有一个PrimitiveObj，也不走拷贝
        if (auto prim = dynamic_cast<PrimitiveObject*>(out_param->spObject.get())) {
            bCloned = false;
        }
        else if (auto lst = dynamic_cast<ListObject*>(out_param->spObject.get())) {
            if (ListHasPrimObj(lst)) {
                bCloned = false;
            }
        }

        bool bAllTaken = false;
        auto outputObj = outNode->getNodeParams().takeOutputObject(out_param, in_param, bAllTaken);
        in_param->spObject = zany2(outputObj->clone());
        in_param->spObject->update_key(stdString2zs(get_uuid_path(m_pNodeRepo)));

        if (outNode->getNodeStatus().is_nocache() && bAllTaken) {
            //似乎要把输入也干掉，但如果一锅清掉，可能会漏了数值输出，或者多对象输出的情况（虽然很少见）
            outNode->getNodeExecutor().mark_takeover();
        }

        if (auto splist = dynamic_cast<ListObject*>(in_param->spObject.get())) {
            update_list_root_key(splist, get_uuid_path(m_pNodeRepo));
        }

        return true;
    }

    // ============================================================
    // formula
    // ============================================================

    float ZNodeExecutor::resolve(const std::string& formulaOrKFrame, const ParamType type)
    {
        const zfxvariant& res = execute_fmla(formulaOrKFrame);
        if (std::holds_alternative<int>(res)) {
            return std::get<int>(res);
        }
        else if (std::holds_alternative<float>(res)) {
            return std::get<float>(res);
        }
        else {
            throw makeNodeError<UnimplError>(get_path(m_pNodeRepo), "the result of formula is not numeric");
        }
        //TODO: kframe issues
        //k帧太麻烦，现阶段用不上先不处理
    }

    std::string ZNodeExecutor::resolve_string(
        const std::string& fmla,
        const std::string& defl)
    {
        try
        {
            //TODO: 要长远解决这类问题，就要拆分“普通编辑框”，单独把公式编辑器拎出来，既可以处理向量的情况，又可以区分字符串是否需要编译
            std::string fmla = defl;
            //先替换掉常见的$F，$ZSG
            if (fmla.find('$') != std::string::npos) {
                auto& sess = zeno::getSession();
                std::filesystem::path pathObj(ws2s(sess.get_project_path()));
                std::filesystem::path dir = pathObj.parent_path();  // 获取上一级目录

                std::unordered_map<std::string, std::string> replacements = {
                    {"$F",   std::to_string(sess.globalState->getFrameId()) },
                    {"$ZSG", dir.string()}
                };
                fmla = replaceTokens(fmla, replacements);
            }

            //只有遇到函数的情况才能解析，目前先不上语法树
            if (fmla.find_first_of("()") != std::string::npos) {
                const zfxvariant& res = execute_fmla(fmla);
                if (std::holds_alternative<std::string>(res)) {
                    return std::get<std::string>(res);
                }
            }
            else {
                return fmla;
            }
        }
        catch (...)
        {
            //再检查一下有没有$F
            if (fmla.find("{$F}") != std::string::npos) {
                int frame = zeno::getSession().globalState->getFrameId();
                std::string res = fmla;
                size_t pos = 0;
                std::string framestr = std::to_string(frame);
                while ((pos = res.find("{$F}", pos)) != std::string::npos) {
                    res.replace(pos, 4, framestr);
                    pos += framestr.length();  // 防止死循环
                }
                return res;
            }
        }
        return defl;
    }

    zfxvariant ZNodeExecutor::execute_fmla(const std::string& expression)
    {
        std::string code = expression;

        ZfxContext ctx;
        ctx.spNode = m_pNodeRepo;
        ctx.spObject = nullptr;
        ctx.code = code;
        ctx.runover = ATTR_GEO;
        ctx.bSingleFmla = true;
        if (!ctx.code.empty() && ctx.code.back() != ';') {
            ctx.code.push_back(';');
        }

        ZfxExecute zfx(ctx.code, &ctx);
        zeno::zfxvariant res = zfx.execute_fmla();
        return res;
    }

    bool ZNodeExecutor::checkAllOutputLinkTraced()
    {
        bool bAllOutputTaken = true;
        //检查是否当前节点是否所有边都被trace了
        for (const auto& [_, outparam] : m_pNodeRepo->getNodeParams().get_output_object_params2()) {
            for (auto link : outparam.links) {
                if (!link->bTraceAndTaken) {
                    bAllOutputTaken = false;
                    break;
                }
            }
        }
        for (const auto& [_, outparam] : m_pNodeRepo->getNodeParams().get_output_prim_params2()) {
            for (auto link : outparam.links) {
                if (!link->bTraceAndTaken) {
                    bAllOutputTaken = false;
                    break;
                }
            }
        }
        return bAllOutputTaken;
    }

    void ZNodeExecutor::launch_param_task(const std::string& param)
    {
        auto& _inputObjs = m_pNodeRepo->getNodeParams().get_input_object_params2();
        auto iter = _inputObjs.find(param);
        if (iter != _inputObjs.end()) {
            ObjectParam& objParam = iter->second;
            if (!objParam.links.empty()) {
                auto spLink = *objParam.links.begin();
                auto& task = spLink->upstream_task;
                if (task.valid()) {
                    //如果没有边，那么spObject在requireInput就会被清空
                    objParam.spObject = task.get();
                }
                else {
                    objParam.spObject = zany2(spLink->fromparam->spObject->clone());
                }
            }
        }
        else {
            auto& _inputPrims = m_pNodeRepo->getNodeParams().get_input_prim_params2();
            auto iter2 = _inputPrims.find(param);
            if (iter2 != _inputPrims.end()) {
                PrimitiveParam& primParam = iter2->second;
                if (primParam.links.size() == 1) {
                    auto spLink = *primParam.links.begin();
                    auto& task = spLink->upstream_task;
                    if (task.valid()) {
                        primParam.result = task.get();
                    }
                }
            }
        }
    }

    bool ZNodeExecutor::requireInput(const std::string& ds, CalcContext* pContext) {
        // 目前假设输入对象和输入数值，不能重名（不难实现，老节点直接改）。
        auto launch_method = zeno::getSession().is_async_executing() ?
            (std::launch::async | std::launch::deferred) : std::launch::deferred;

        if (ds == "path") {
            int j;
            j = 0;
        }

        auto& nodeParams = m_pNodeRepo->getNodeParams();
        //虽然划分了职责，但其实这里还是得耦合掉底层的逻辑。
        auto& inputObjParams = nodeParams.get_input_object_params2();

        auto iter = inputObjParams.find(ds);
        if (iter != inputObjParams.end()) {
            ObjectParam* in_param = &(iter->second);
            if (in_param->links.empty()) {
                //节点如果定义了对象，但没有边连上去，是否要看节点apply如何处理？
                //FIX: 没有边的情况要清空掉对象，否则apply以为这个参数连上了对象
                in_param->spObject.reset();
            }
            else {
                if (in_param->links.empty()) {
                    //清空缓存对象
                    in_param->spObject.reset();
                }

                //改为异步计算以后，直接发起task即可，后续再考虑组装的事宜
                for (auto spLink : in_param->links) {
                    ObjectParam* out_param = spLink->fromparam;
                    auto outNode = out_param->m_wpNode;
                    if (outNode->getNodeExecutor().is_takenover()) {
                        //维持原来的参数结果状态，不从上游取（事实上上游的内容已经被删掉了）
                        return true;
                    }
                    if (outNode->getNodeExecutor().is_dirty())
                    {
                        ExecuteContext ctx;
                        ctx.in_node = get_name(m_pNodeRepo);
                        ctx.in_param = in_param->name;
                        ctx.out_param = out_param->name;
                        ctx.pContext = pContext;
                        ctx.innode_uuid_path = get_uuid_path(m_pNodeRepo);

                        //发起异步任务：
                        ZNodeExecutor* pExecutor = &(outNode->getNodeExecutor());
                        spLink->upstream_task = std::async(launch_method, &ZNodeExecutor::execute_get_object, pExecutor, ctx);
                    }
                }
            }
        }
        else {
            auto& inputPrimsParams = nodeParams.get_input_prim_params2();
            auto iter2 = inputPrimsParams.find(ds);
            if (iter2 != inputPrimsParams.end()) {
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
                            Graph* spSrcGraph = spSrcNode->getNodeStatus().getGraph();
                            assert(spSrcGraph);
                            //NOTE in 2025/3/25: 还是apply引用源，至于本节点参数循环引用的问题，走另外的路线
                            //TODO: refactor with async tasks
                            if (spSrcNode == m_pNodeRepo) {
                                //引用自身的参数，直接拿defl，因为这种情况绝大多数是固定值，没必要执行计算，比如pos引用size的数据
                                spSrcNode->getNodeExecutor().doApply_Parameter(reflink->source_inparam->name, pContext);
                            }
                            else {
                                spSrcNode->getNodeExecutor().doApply(pContext);
                            }
                        }
                    }

                    const zeno::reflect::Any& primval = processPrimitive(in_param);
                    in_param->result = primval;
                    //旧版本的requireInput指的是是否有连线，如果想兼容旧版本，这里可以返回false，但使用量不多，所以就修改它的定义。
                }
                else {
                    for (auto spLink : in_param->links) {
                        auto outNode = spLink->fromparam->m_wpNode;
                        if (outNode->getNodeExecutor().is_takenover()) {
                            continue;
                        }
                        if (outNode->getNodeExecutor().is_dirty())
                        {
                            ExecuteContext ctx;
                            ctx.in_node =  get_name(m_pNodeRepo);
                            ctx.in_param = in_param->name;
                            ctx.out_param = spLink->fromparam->name;
                            ctx.pContext = pContext;
                            ctx.innode_uuid_path = get_uuid_path(m_pNodeRepo);
                            spLink->upstream_task = std::async(launch_method, &ZNodeExecutor::execute_get_numeric, &(outNode->getNodeExecutor()), ctx);
                        }
                        else {
                            //直接从上游拷过来即可
                            in_param->result = spLink->fromparam->result;
                        }
                    }

                }
            }
            else {
                return false;
            }
        }
        return true;
    }

    void ZNodeExecutor::preApply(CalcContext* pContext)
    {
        if (!m_dirty)
            return;

        //debug
#if 1
        if (get_name(m_pNodeRepo) == "FormSceneTree1") {
            int j;
            j = 0;
        }
#endif

        reportStatus(true, Node_Pending);

        auto& _inputObjs = m_pNodeRepo->getNodeParams().get_input_object_params2();
        auto& _inputPrims = m_pNodeRepo->getNodeParams().get_input_prim_params2();

        //TODO: the param order should be arranged by the descriptors.
        for (const auto& [name, param] : _inputObjs) {
            bool ret = requireInput(name, pContext);
            if (!ret)
                zeno::log_warn("the param {} may not be initialized", name);
        }
        for (const auto& [name, param] : _inputPrims) {
            bool ret = requireInput(name, pContext);
            if (!ret)
                zeno::log_warn("the param {} may not be initialized", name);
        }

        //wait all
        for (auto& [name, param] : _inputObjs) {
            for (auto link : param.links) {
                auto& task = link->upstream_task;
                if (task.valid()) {
                    task.wait();
                }
            }
        }
        for (auto& [name, param] : _inputPrims) {
            for (auto link : param.links) {
                auto& task = link->upstream_task;
                if (task.valid()) {
                    task.wait();
                }
            }
        }

        //resolve all dependencys for input params
        for (auto& [name, param] : _inputObjs) {
            if (param.type == gParamType_List) {
                param.spObject = processList(&param, pContext);
            }
            else {
                if (param.links.size() == 1) {
                    auto spLink = *param.links.begin();
                    auto& task = spLink->upstream_task;
                    if (task.valid()) {
                        param.spObject = task.get();
                    }
                    else {
                        if (!spLink->fromparam->spObject)
                            throw makeNodeError<UnimplError>(get_path(m_pNodeRepo), "cannot get object from upstream");

                        //task为空，因为之前已经被执行过一次了，上游已经算好了
                        //没有task，说明上游不需要计算，但对于多支路的下游来说，就不一定知道上游是否经历了计算
                        //因此，在传递脏位的时候，要顺带把param.spObject情况，然而再到 上游拷一次
                        if (!param.spObject)
                            param.spObject = zany2(spLink->fromparam->spObject->clone());
                    }
                    param.spObject->update_key(stdString2zs(m_pNodeRepo->get_uuid()));
                }
            }
        }

        for (auto& [name, param] : _inputPrims) {
            if (param.type != gParamType_ListOfMat4) {
                //如果不是list类型，只会有一个连接
                if (!param.links.empty()) {
                    auto spLink = *param.links.begin();
                    auto& task = spLink->upstream_task;
                    if (task.valid()) {
                        param.result = task.get();
                    }
                    else {
                        //直接从上游拷过来即可
                        param.result = spLink->fromparam->result;
                    }
                }
            }
            else {
                std::vector<glm::mat4> result;
                for (auto spLink : param.links) {
                    auto& task = spLink->upstream_task;
                    zeno::reflect::Any res;
                    if (task.valid()) {
                        res = task.get();
                    }
                    else {
                        res = spLink->fromparam->result;
                    }
                    if (res.type().hash_code() == gParamType_ListOfMat4) {
                        result = zeno::reflect::any_cast<std::vector<glm::mat4>>(res);
                        break;
                    }
                    else if (res.type().hash_code() == gParamType_Matrix4) {
                        result.push_back(zeno::reflect::any_cast<glm::mat4>(res));
                    }
                    else {
                        throw makeNodeError<UnimplError>(get_path(m_pNodeRepo), "");
                    }
                }
                param.result = result;
            }
        }
    }

    void ZNodeExecutor::preApply_Primitives(CalcContext* pContext)
    {
        auto& _inputPrims = m_pNodeRepo->getNodeParams().get_input_prim_params2();

        if (!m_dirty)
            return;
        for (const auto& [name, param] : _inputPrims) {
            bool ret = requireInput(name, pContext);
            if (!ret)
                zeno::log_warn("the param {} may not be initialized", name);
        }

        for (auto& [name, param] : _inputPrims) {
            if (param.type != gParamType_ListOfMat4) {
                //如果不是list类型，只会有一个连接
                if (!param.links.empty()) {
                    auto& task = (*param.links.begin())->upstream_task;
                    if (task.valid()) {
                        param.result = task.get();
                    }
                }
            }
            else {
                //TODO:
            }
        }
    }

    void ZNodeExecutor::preApply_SwitchIf(CalcContext* pContext)
    {
        preApply_Primitives(pContext);

        const zeno::reflect::Any& res = m_pNodeRepo->getNodeParams().get_param_result("Condition");
        int cond = 0;
        if (res.type().hash_code() == gParamType_Int) {
            cond = zeno::reflect::any_cast<int>(res);
        }
        else if (res.type().hash_code() == gParamType_Float) {
            cond = zeno::reflect::any_cast<float>(res);
        }
        else if (res.type().hash_code() == gParamType_Bool) {
            cond = zeno::reflect::any_cast<bool>(res);
        }

        const std::string& exec_param = (cond != 0) ? "If True" : "If False";
        requireInput(exec_param, pContext);
        launch_param_task(exec_param);
    }

    void ZNodeExecutor::preApply_SwitchBetween(CalcContext* pContext)
    {
        preApply_Primitives(pContext);

        int cond = zeno::reflect::any_cast<int>(m_pNodeRepo->getNodeParams().get_param_result("cond1"));
        if (cond != 0) {
            requireInput("b1", pContext);
            launch_param_task("b1");
            return;
        }

        cond = zeno::reflect::any_cast<int>(m_pNodeRepo->getNodeParams().get_param_result("cond2"));
        if (cond != 0) {
            requireInput("b2", pContext);
            launch_param_task("b2");
            return;
        }

        cond = zeno::reflect::any_cast<int>(m_pNodeRepo->getNodeParams().get_param_result("cond3"));
        if (cond != 0) {
            requireInput("b3", pContext);
            launch_param_task("b3");
            return;
        }

        cond = zeno::reflect::any_cast<int>(m_pNodeRepo->getNodeParams().get_param_result("cond4"));
        if (cond != 0) {
            requireInput("b4", pContext);
            launch_param_task("b4");
            return;
        }

        cond = zeno::reflect::any_cast<int>(m_pNodeRepo->getNodeParams().get_param_result("cond5"));
        if (cond != 0) {
            requireInput("b5", pContext);
            launch_param_task("b5");
            return;
        }

        cond = zeno::reflect::any_cast<int>(m_pNodeRepo->getNodeParams().get_param_result("cond6"));
        if (cond != 0) {
            requireInput("b6", pContext);
            launch_param_task("b6");
            return;
        }

        cond = zeno::reflect::any_cast<int>(m_pNodeRepo->getNodeParams().get_param_result("cond7"));
        if (cond != 0) {
            requireInput("b7", pContext);
            launch_param_task("b7");
            return;
        }

        cond = zeno::reflect::any_cast<int>(m_pNodeRepo->getNodeParams().get_param_result("cond8"));
        if (cond != 0) {
            requireInput("b8", pContext);
            launch_param_task("b8");
            return;
        }
    }

    void ZNodeExecutor::preApply_FrameCache(CalcContext* pContext)
    {
        //这里有一个问题，就是如果应用重启了，如何判定是否需要重新写cache?
        if (m_dirtyReason == Dirty_All) {
            preApply(pContext);
        }
        else if (m_dirtyReason == Dirty_FrameChanged) {
            preApply_Primitives(pContext);
            //需要把路径拿出来，才得知cache是否已经存在了，如果cache不存在，
            //即便framechange也得把上游依赖解了
            auto param = m_pNodeRepo->getNodeParams().get_input_prim_param("path");
            auto path = zeno::reflect::any_cast<std::string>(param.result);
            if (!std::filesystem::exists(path)) {
                preApply(pContext);
            }
        }
    }

    void ZNodeExecutor::bypass()
    {
        //在preAppy拷贝了对象以后再直接赋值给output，还有一种方法是不拷贝，直接把上一个节点的输出给到这里的输出。

        //找到输入和输出的唯一object(如果输入有两个，并且有一个有连线，是否采纳连线这个？）
        //不考虑数值类型的输出
        auto& _outputObjs = m_pNodeRepo->getNodeParams().get_output_object_params2();
        auto& _inputObjs = m_pNodeRepo->getNodeParams().get_input_object_params2();
        if (_outputObjs.empty() || _inputObjs.empty()) {
            throw makeNodeError<UnimplError>(get_path(m_pNodeRepo), "there is not matched input and output object when mute button is on");
        }
        ObjectParam& input_objparam = _inputObjs.begin()->second;
        ObjectParam& output_objparam = _outputObjs.begin()->second;
        if (input_objparam.type != output_objparam.type) {
            throw makeNodeError<UnimplError>(get_path(m_pNodeRepo), "the input and output type is not matched, when the mute button is on");
        }
        output_objparam.spObject = zany2(input_objparam.spObject->clone());
    }

    // ============================================================
    // timeshift / foreach
    // ============================================================

    void ZNodeExecutor::preApplyTimeshift(CalcContext* pContext)
    {
        int oldFrame = getSession().globalState->getFrameId();
        scope_exit sp([&oldFrame] { getSession().globalState->updateFrameId(oldFrame); });
        //get offset
        auto defl = m_pNodeRepo->getNodeParams().get_input_prim_param("offset").defl;
        zeno::PrimVar offset = defl.has_value() ? zeno::reflect::any_cast<zeno::PrimVar>(defl) : 0;
        int newFrame = oldFrame + std::get<int>(offset);
        //clamp
        auto startFrameDefl = m_pNodeRepo->getNodeParams().get_input_prim_param("start frame").defl;
        int globalStartFrame = getSession().globalState->getStartFrame();
        int startFrame = startFrameDefl.has_value() ? std::get<int>(zeno::reflect::any_cast<PrimVar>(startFrameDefl)) : globalStartFrame;
        auto endFrameDefl = m_pNodeRepo->getNodeParams().get_input_prim_param("end frame").defl;
        int globalEndFrame = getSession().globalState->getEndFrame();
        int endFrame = endFrameDefl.has_value() ? std::get<int>(zeno::reflect::any_cast<PrimVar>(endFrameDefl)) : globalEndFrame;
        auto clampDefl = m_pNodeRepo->getNodeParams().get_input_prim_param("clamp").defl;
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
        propagateDirty(m_pNodeRepo, "$F");
        preApply(pContext);
    }

    void ZNodeExecutor::foreachend_apply(CalcContext* pContext)
    {
        //当前节点是ForeachEnd
        std::string foreach_begin_path = zeno::any_cast_to_string(m_pNodeRepo->getNodeParams().get_defl_value("ForEachBegin Path"));
        if (Graph* spGraph = m_pNodeRepo->getNodeStatus().getGraph()) {
            auto foreach_begin = spGraph->getNode(foreach_begin_path);
            auto foreach_end = static_cast<ForEachEnd*>(coreNode());
            assert(foreach_end);
            for (foreach_end->reset_forloop_settings(m_pNodeRepo);
                foreach_end->is_continue_to_run(m_pNodeRepo, pContext);
                foreach_end->increment(m_pNodeRepo))
            {
                foreach_begin->getNodeExecutor().mark_dirty(true);
                //pContext->curr_iter = zeno::reflect::any_cast<int>(foreach_begin->get_defl_value("Current Iteration"));
                preApply(pContext);
                foreach_end->apply_foreach(&m_pNodeRepo->getNodeParams(), pContext);
            }
            auto output = m_pNodeRepo->getNodeParams().get_output_obj("Output Object");
            if (output) {
                output->update_key(stdString2zs(m_pNodeRepo->get_uuid()));
            }
        }
    }

} // namespace zeno

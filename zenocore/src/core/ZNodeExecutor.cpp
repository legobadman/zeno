#include <zeno/core/ZNodeExecutor.h>
#include <utility>

namespace zeno {

    ZNodeExecutor::ZNodeExecutor(ZNode* pNodeRepo, INode2* pNode, void (*dtor)(INode2*))
        : m_upNode2(pNode, dtor)
        , m_pNodeRepo(pNodeRepo)
    {
    }

    // ---------------- core execute ----------------

    void ZNodeExecutor::execute(CalcContext* pContext)
    {
        (void)pContext;
    }

    zany2 ZNodeExecutor::execute_get_object(const ExecuteContext& exec_context)
    {
        (void)exec_context;
        return {};
    }

    zeno::reflect::Any ZNodeExecutor::execute_get_numeric(const ExecuteContext& exec_context)
    {
        (void)exec_context;
        return {};
    }

    void ZNodeExecutor::doOnlyApply()
    {
    }

    void ZNodeExecutor::mark_dirty(bool bOn, DirtyReason reason, bool bWholeSubnet, bool bRecursively)
    {
        (void)bOn;
        (void)reason;
        (void)bWholeSubnet;
        (void)bRecursively;
    }

    void ZNodeExecutor::mark_clean()
    {
    }

    void ZNodeExecutor::dirty_changed(bool bOn, DirtyReason reason, bool bWholeSubnet, bool bRecursively)
    {
        (void)bOn;
        (void)reason;
        (void)bWholeSubnet;
        (void)bRecursively;
    }

    void ZNodeExecutor::clearCalcResults()
    {
    }

    float ZNodeExecutor::time() const
    {
        return 0.f;
    }

    // ---------------- foreach support ----------------

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

    // ---------------- state helpers ----------------

    void ZNodeExecutor::onInterrupted()
    {
    }

    void ZNodeExecutor::mark_previous_ref_dirty()
    {
    }

    void ZNodeExecutor::update_out_objs_key()
    {
    }

    void ZNodeExecutor::mark_dirty_objs()
    {
    }

    void ZNodeExecutor::reportStatus(bool bDirty, NodeRunStatus status)
    {
        (void)bDirty;
        (void)status;
    }

    void ZNodeExecutor::mark_takeover()
    {
    }

    bool ZNodeExecutor::is_takenover() const
    {
        return false;
    }

    void ZNodeExecutor::check_break_and_return()
    {
    }

    void ZNodeExecutor::complete()
    {
    }

    void ZNodeExecutor::apply()
    {
    }

    // ---------------- apply internals ----------------

    void ZNodeExecutor::doApply(CalcContext* pContext)
    {
        (void)pContext;
    }

    void ZNodeExecutor::doApply_Parameter(std::string const& name, CalcContext* pContext)
    {
        (void)name;
        (void)pContext;
    }

    zeno::reflect::Any ZNodeExecutor::processPrimitive(PrimitiveParam* in_param)
    {
        (void)in_param;
        return {};
    }

    std::unique_ptr<ListObject> ZNodeExecutor::processList(ObjectParam* in_param, CalcContext* pContext)
    {
        (void)in_param;
        (void)pContext;
        return nullptr;
    }

    bool ZNodeExecutor::receiveOutputObj(ObjectParam* in_param, NodeImpl* outNode, ObjectParam* out_param)
    {
        (void)in_param;
        (void)outNode;
        (void)out_param;
        return false;
    }

    float ZNodeExecutor::resolve(const std::string& formulaOrKFrame, const ParamType type)
    {
        (void)formulaOrKFrame;
        (void)type;
        return 0.f;
    }

    std::string ZNodeExecutor::resolve_string(const std::string& fmla, const std::string& defl)
    {
        (void)fmla;
        return defl;
    }

    zfxvariant ZNodeExecutor::execute_fmla(const std::string& expression)
    {
        (void)expression;
        return {};
    }

    std::set<RefSourceInfo> ZNodeExecutor::resolveReferSource(const zeno::reflect::Any& param_defl)
    {
        (void)param_defl;
        return {};
    }

    void ZNodeExecutor::initReferLinks(PrimitiveParam* target_param)
    {
        (void)target_param;
    }

    bool ZNodeExecutor::checkAllOutputLinkTraced()
    {
        return false;
    }

    void ZNodeExecutor::launch_param_task(const std::string& param)
    {
        (void)param;
    }

    // ---------------- preApply ----------------

    void ZNodeExecutor::preApply(CalcContext* pContext)
    {
        (void)pContext;
    }

    void ZNodeExecutor::preApply_Primitives(CalcContext* pContext)
    {
        (void)pContext;
    }

    void ZNodeExecutor::preApply_SwitchIf(CalcContext* pContext)
    {
        (void)pContext;
    }

    void ZNodeExecutor::preApply_SwitchBetween(CalcContext* pContext)
    {
        (void)pContext;
    }

    void ZNodeExecutor::preApply_FrameCache(CalcContext* pContext)
    {
        (void)pContext;
    }

    void ZNodeExecutor::bypass()
    {
    }

    // ---------------- timeshift / foreach ----------------

    void ZNodeExecutor::preApplyTimeshift(CalcContext* pContext)
    {
        (void)pContext;
    }

    void ZNodeExecutor::foreachend_apply(CalcContext* pContext)
    {
        (void)pContext;
    }

} // namespace zeno

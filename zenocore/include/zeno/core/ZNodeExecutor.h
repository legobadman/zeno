#pragma once

#include <zeno/utils/api.h>
#include <iobject2.h>
#include <inodedata.h>
#include <zeno/utils/safe_dynamic_cast.h>
#include <zeno/funcs/LiterialConverter.h>
#include <variant>
#include <memory>
#include <string>
#include <set>
#include <map>
#include <zeno/types/ListObject.h>
#include <zeno/extra/GlobalState.h>
#include <zeno/formula/syntax_tree.h>
#include <zeno/core/data.h>
#include <zeno/utils/uuid.h>
#include <zeno/utils/safe_at.h>
#include <zeno/core/CoreParam.h>
#include <functional>
#include <reflect/registry.hpp>
#include <zcommon.h>
#include <inodedata.h>
#include <inodeimpl.h>
#include <zeno/core/ZNode.h>
#include <zeno/extra/CalcContext.h>
#include <zeno/core/FunctionManager.h>


namespace zeno {

    struct ExecuteContext
    {
        std::string innode_uuid_path;
        std::string in_node;
        std::string in_param;
        std::string out_param;
        CalcContext* pContext;
    };

    class ZNodeExecutor {
    public:
        ZNodeExecutor() = delete;
        ZNodeExecutor(ZNode* pNodeRepop, INode2* pNode, void (*dtor)(INode2*));
        ZNodeExecutor(const ZNodeExecutor&) = delete;

        void execute(CalcContext* pContext);
        zany2 execute_get_object(const ExecuteContext& exec_context);
        zeno::reflect::Any execute_get_numeric(const ExecuteContext& exec_context);
        void doOnlyApply();
        void mark_dirty(
            bool bOn,
            DirtyReason reason = zeno::Dirty_All,
            bool bWholeSubnet = true,
            bool bRecursively = true
        );
        virtual void mark_clean();
        virtual void dirty_changed(bool bOn, DirtyReason reason, bool bWholeSubnet, bool bRecursively);
        virtual void clearCalcResults();
        virtual float time() const;

        //foreach特供
        virtual bool is_continue_to_run();
        virtual void increment();
        virtual void reset_forloop_settings();
        virtual std::shared_ptr<IObject2> get_iterate_object();

        void onInterrupted();
        void mark_previous_ref_dirty();
        void update_out_objs_key();
        void mark_dirty_objs();
        void reportStatus(bool bDirty, NodeRunStatus status);
        void mark_takeover();
        bool is_takenover() const;
        void check_break_and_return();
        void complete();
        void apply();

        void doApply(CalcContext* pContext);
        void doApply_Parameter(std::string const& name, CalcContext* pContext); //引入数值输入参数，并不计算整个节点
        zeno::reflect::Any processPrimitive(PrimitiveParam* in_param);
        std::unique_ptr<ListObject> processList(ObjectParam* in_param, CalcContext* pContext);
        bool receiveOutputObj(ObjectParam* in_param, NodeImpl* outNode, ObjectParam* out_param);
        float resolve(const std::string& formulaOrKFrame, const ParamType type);
        std::string resolve_string(const std::string& fmla, const std::string& defl);
        zfxvariant execute_fmla(const std::string& expression);
        template<class T, class E> T resolveVec(const zeno::reflect::Any& defl, const ParamType type);
        bool checkAllOutputLinkTraced();
        void launch_param_task(const std::string& param);

        //preApply是先解决所有输入参数（上游）的求值问题
        void preApply(CalcContext* pContext);
        void preApply_Primitives(CalcContext* pContext);
        void preApply_SwitchIf(CalcContext* pContext);
        void preApply_SwitchBetween(CalcContext* pContext);
        void preApply_FrameCache(CalcContext* pContext);
        void bypass();

        //for timeshift node
        void preApplyTimeshift(CalcContext* pContext);
        //foreach特供
        void foreachend_apply(CalcContext* pContext);

    private:
        std::unique_ptr<INode2, void (*)(INode2*)> m_upNode2;
        ZNode* m_pNodeRepo{};
        NodeRunStatus m_status = Node_DirtyReadyToRun;
        DirtyReason m_dirtyReason = NoDirty;
    };
}
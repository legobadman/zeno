#pragma once

#include <zeno/core/ZNode.h>
#include <zeno/zeno.h>
#include <zeno/core/Graph.h>
#include <zeno/types/PrimitiveObject.h>
#include <zeno/types/ListObject_impl.h>
#include <zeno/formula/zfxexecute.h>
#include <zeno/core/FunctionManager.h>
#include <zeno/types/GeometryObject.h>
#include <zeno/utils/helper.h>
#include <zeno/utils/interfaceutil.h>
#include <zeno/geo/geometryutil.h>


namespace zeno
{
    struct ForEachBegin : INode2
    {
        ZNode* get_foreachend(ZNode* m_pAdapter);
        ZErrorCode apply(INodeData* ptrNodeData) override;
        NodeType type() const override { return Node_Normal; }
        void clearCalcResults() override {}
        float time() const override { return 1.0; }
        int get_current_iteration(ZNode* m_pAdapter);
        void update_iteration(ZNode* m_pAdapter, int new_iteration);
    };

    struct ForEachEnd : INode2
    {
        NodeType type() const override { return Node_Normal; }
        float time() const override { return 1.0; }

        ForEachEnd();
        ZNode* get_foreach_begin(ZNode* m_pAdapter);
        void reset_forloop_settings(ZNode* m_pAdapter);
        bool is_continue_to_run(ZNode* m_pAdapter, CalcContext* pContext);
        void increment(ZNode* m_pAdapter);
        IObject2* get_iterate_object();
        ZErrorCode apply(INodeData* ptrNodeData) override;
        void apply_foreach(INodeData* ptrNodeData, CalcContext* pContext);
        void adjustCollectObjInfo(ZNode* ptrNodeData);
        void clearCalcResults() override;

        zany2 m_iterate_object;
        std::unique_ptr<ListObject> m_collect_objs;     //TODO: 如果foreach的对象是Dict，但这里收集的结果将会以list返回出去，以后再支持Dict的收集
        //std::vector<IObject2*> m_last_collect_objs;    //直接储存raw pointer危险
    };
}
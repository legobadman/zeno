#pragma once

#include <zeno/core/NodeImpl.h>
#include <zeno/zeno.h>
#include <zeno/core/Graph.h>
#include <zeno/types/PrimitiveObject.h>
#include <zeno/types/DictObject.h>
#include <zeno/types/ListObject_impl.h>
#include <zeno/formula/zfxexecute.h>
#include <zeno/core/FunctionManager.h>
#include <zeno/types/GeometryObject.h>
#include <zeno/utils/helper.h>
#include <zeno/types/IGeometryObject.h>
#include <zeno/utils/interfaceutil.h>
#include <zeno/geo/geometryutil.h>


namespace zeno
{
    struct ForEachBegin : INode
    {
        INode* get_foreachend();
        void apply() override;
        int get_current_iteration();
        void update_iteration(int new_iteration);
    };

    struct ForEachEnd : INode
    {
        ForEachEnd();
        ForEachBegin* get_foreach_begin();
        void reset_forloop_settings();
        bool is_continue_to_run();
        void increment();
        zany get_iterate_object();
        void apply() override;
        void apply_foreach(CalcContext* pContext);
        void adjustCollectObjInfo();

        zany m_iterate_object;
        zeno::SharedPtr<ListObject> m_collect_objs;     //TODO: ���foreach�Ķ�����Dict���������ռ��Ľ��������list���س�ȥ���Ժ���֧��Dict���ռ�
        std::vector<zany> m_last_collect_objs;
    };
}
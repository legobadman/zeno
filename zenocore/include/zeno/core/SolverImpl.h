#pragma once

#include <zeno/core/NodeImpl.h>


namespace zeno
{
    class SolverImpl : public NodeImpl
    {
    public:
        SolverImpl(INode2* pNode, void (*dtor)(INode2*));
        virtual ~SolverImpl();
        void terminate_solve();
        IObject2* get_default_output_object();
        void clearCalcResults() override;
        void dirty_changed(bool bOn, DirtyReason reason, bool bWholeSubnet, bool bRecursively) override;
    };
}
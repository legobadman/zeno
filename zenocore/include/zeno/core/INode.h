#pragma once

#include <zeno/core/IObject.h>
#include <zeno/core/data.h>

namespace zeno
{
    //struct IObject;
    struct INodeImpl;

    struct INode {
        virtual void apply() = 0;

        //foreach issues.
        virtual bool is_continue_to_run() { return false; }
        virtual void increment() {}
        virtual void reset_forloop_settings() {}
        virtual zany get_iterate_object() { return 0; }
        virtual CustomUI export_customui() const { return CustomUI(); }

        INodeImpl* m_pAdapter;
    };

#if 0
    struct TestNode : INode {
        void apply() override {
            IObject* pObject = m_pAdapter->get_input_object("prim");
            m_pAdapter->set_output_object("prim", pObject);
        }
    };

    static struct _DefImplNode {
        _DefImplNode(::zeno::Descriptor const& desc) {
            ::zeno::getSession().defNodeClass3(
                []() -> INode* { return new TestNode(); },
                "TestNode",
                desc
            );
        }
    }_defImplNode;
#endif
}


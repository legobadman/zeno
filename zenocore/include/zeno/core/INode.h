#pragma once

#include <zeno/core/IObject.h>
#include <zeno/core/data.h>
#include <zeno/core/INodeImpl.h>

namespace zeno
{
    struct INode {
        virtual void apply() = 0;
        virtual CustomUI export_customui() const { return m_pAdapter->export_customui(); }

        zany get_input(const zeno::String& param)            { return m_pAdapter->get_input_object(param); }
        zeno::SharedPtr<PrimitiveObject> get_input_PrimitiveObject(const zeno::String& param) { return m_pAdapter->get_input_PrimitiveObject(param); }
        int get_input2_int(const zeno::String& param)               { return m_pAdapter->get_input2_int(param); }
        float get_input2_float(const zeno::String& param)           { return m_pAdapter->get_input2_float(param); }
        String get_input2_string(const zeno::String& param)         { return m_pAdapter->get_input2_string(param);}
        zeno::Vec3f get_input2_vec3f(const zeno::String& param)     { return m_pAdapter->get_input2_vec3f(param); }
        bool get_input2_bool(const zeno::String& param)             { return m_pAdapter->get_input2_bool(param); }
        bool has_input(const zeno::String& param)                   { return m_pAdapter->has_input(param); }
        void set_output(const zeno::String& param, zany pObject)    { return m_pAdapter->set_output_object(param, pObject); }

        //foreach issues.
        virtual bool is_continue_to_run() { return false; }
        virtual void increment() {}
        virtual void reset_forloop_settings() {}
        virtual zany get_iterate_object() { return 0; }

        INodeImpl* m_pAdapter = nullptr;
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


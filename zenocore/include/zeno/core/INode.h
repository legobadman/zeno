#pragma once

#include <zeno/core/IObject.h>
#include <zeno/core/data.h>

namespace zeno
{
    struct NodeImpl;
    struct ListObject;
    struct DictObject;
    struct PrimitiveObject;
    struct GlobalState;

    struct ZENO_API INode {
        virtual void apply() = 0;
        virtual CustomUI export_customui() const;
        virtual NodeType type() const;
        virtual ~INode() = default;     //暂时不考虑abi问题

        zany get_input(const zeno::String& param);
        zany get_output(const zeno::String& param);
        zeno::SharedPtr<PrimitiveObject> get_input_PrimitiveObject(const zeno::String& param);
        zeno::SharedPtr<ListObject> get_input_ListObject(const zeno::String& param);
        zeno::SharedPtr<DictObject> get_input_DictObject(const zeno::String& param);
        int get_input2_int(const zeno::String& param);
        float get_input2_float(const zeno::String& param);
        String get_input2_string(const zeno::String& param);
        bool get_input2_bool(const zeno::String& param);
        bool has_input(const zeno::String& param);

        zeno::Vec2i get_input2_vec2i(const zeno::String& param);
        zeno::Vec2f get_input2_vec2f(const zeno::String& param);
        zeno::Vec3i get_input2_vec3i(const zeno::String& param);
        zeno::Vec3f get_input2_vec3f(const zeno::String& param);
        zeno::Vec4i get_input2_vec4i(const zeno::String& param);
        zeno::Vec4f get_input2_vec4f(const zeno::String& param);

        int get_param_int(const zeno::String& param);
        float get_param_float(const zeno::String& param);
        String get_param_string(const zeno::String& param);
        bool get_param_bool(const zeno::String& param);

        bool set_output(const zeno::String& param, zany pObject);
        void set_output_int(const zeno::String& param, int val);
        void set_output_float(const zeno::String& param, float val);
        void set_output_string(const zeno::String& param, zeno::String val);
        void set_output_vec2f(const zeno::String& param, Vec2f val);
        void set_output_vec2i(const zeno::String& param, Vec2i val);
        void set_output_vec3f(const zeno::String& param, Vec3f val);
        void set_output_vec3i(const zeno::String& param, Vec3i val);

        int GetFrameId() const;
        GlobalState* getGlobalState();

        //foreach issues.
        virtual bool is_continue_to_run();
        virtual void increment();
        virtual void reset_forloop_settings();
        virtual zany get_iterate_object();

        NodeImpl* m_pAdapter = nullptr;
    };
}


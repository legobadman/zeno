#pragma once

#include <zeno/core/coredata.h>
#include <zeno/core/INode.h>


namespace zeno
{
    struct NodeImpl;
    struct ListObject;
    struct DictObject;
    struct PrimitiveObject;

    #define IMPL(p) (p)->m_pImpl

    struct ZENO_API INodeImpl
    {
        INodeImpl(INode* pNode);

        zany get_input_object(const zeno::String& param);

        zeno::SharedPtr<PrimitiveObject> get_input_PrimitiveObject(const zeno::String& param);
        zeno::SharedPtr<ListObject> get_input_ListObject(const zeno::String& param);
        zeno::SharedPtr<DictObject> get_input_DictObject(const zeno::String& param);
        //TODO: VDB, NumericObject

        void set_output_object(const zeno::String& param, zany pObject);

        int get_input2_int(const zeno::String& param);
        float get_input2_float(const zeno::String& param);
        zeno::String get_input2_string(const zeno::String& param);
        zeno::Vec2i get_input2_vec2i(const zeno::String& param);
        zeno::Vec2f get_input2_vec2f(const zeno::String& param);
        zeno::Vec3i get_input2_vec3i(const zeno::String& param);
        zeno::Vec3f get_input2_vec3f(const zeno::String& param);
        zeno::Vec4f get_input2_vec4i(const zeno::String& param);
        zeno::Vec4i get_input2_vec4f(const zeno::String& param);
        bool get_input2_bool(const zeno::String& param);
        bool has_input(const zeno::String& param);

        NodeImpl* m_pImpl;
    };


}

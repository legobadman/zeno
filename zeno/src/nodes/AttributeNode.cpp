#include <zeno/zeno.h>
#include <zeno/core/reflectdef.h>
#include "zeno_types/reflect/reflection.generated.hpp"


namespace zeno {

    using namespace zeno::reflect;

    struct ZDEFNODE() CreateAttribute : INode {

        ReflectCustomUI m_uilayout = {
            _Group {
                {"input_object", ParamObject("Input Object", Socket_Clone)},
                {"type", ParamPrimitive("Attribute Type", "Vector3", Combobox, std::vector<std::string>{
                    "Vector2",
                    "Vector3",
                    "Vector4",
                    "Integer",
                    "Float",
                    "String",
                    "Boolean"})},
                {"value", ParamPrimitive("Value", zeno::vec3f(0,0,0), Vec3edit, Any(), "",
                    "string current_type = param('Attribute Type').value;\
                     if (current_type == 'Integer') {\
                         control = 'Lineedit';\
                         type = 'string';\
                     }\
                     else if (current_type == 'Float') {\
                         control = 'Lineedit';\
                         type = 'float';\
                     }\
                     else if (current_type == 'Boolean') {\
                         control = 'Checkbox';\
                         type = 'boolean';\
                     }\
                     else if (current_type == 'String') {\
                         control = 'Lineedit';\
                         type = 'string';\
                     }\
                     else if (current_type == 'Vector3') {\
                         control = 'Vec3edit';\
                         type = 'vec3f';\
                     }\
                     else if (current_type == 'Vector2') {\
                         control = 'Vec2edit';\
                         type = 'vec2f';\
                     }\
                     else if (current_type == 'Vector4') {\
                         control = 'Vec4edit';\
                         type = 'vec4f';\
                     }\
                    ")},
                {"attr_group", ParamPrimitive("Attribute Group", "Point", Combobox, std::vector<std::string>{
                        "Point",
                        "Face",
                        "Geometry"})
                },
            },
            _Group {
                {"", ParamObject("Output Object")},
            }
        };

        zany apply(zany input_object, std::string type, Any value, std::string attr_group) {
            return input_object;
        }
    };
}


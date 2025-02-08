#include <zeno/zeno.h>
#include <zeno/core/reflectdef.h>
#include <zeno/types/GeometryObject.h>
#include "zeno_types/reflect/reflection.generated.hpp"


namespace zeno {

    using namespace zeno::reflect;

    struct ZDEFNODE() CreateAttribute : INode {

        ReflectCustomUI m_uilayout = {
            _Group {
                {"input_object", ParamObject("Input", Socket_Clone)},
            },
            _Group {
                {"", ParamObject("Output")},
            }
        };

        zany apply(zany input_object, std::string attr_name = "attribute1") {

            if (attr_name == "") {
                throw makeError<UnimplError>("the attribute name cannot be empty.");
            }

            AttrVar attr_value;
            if (m_type == "Vector2") {
                attr_value = m_vec2Value;
            }
            else if (m_type == "Vector3") {
                attr_value = m_vec3Value;
            }
            else if (m_type == "Vector4") {
                attr_value = m_vec4Value;
            }
            else if (m_type == "Integer") {
                attr_value = m_intValue;
            }
            else if (m_type == "Float") {
                attr_value = m_floatValue;
            }
            else if (m_type == "String") {
                attr_value = m_stringValue;
            }
            else if (m_type == "Boolean") {
                attr_value = m_boolValue;
            }

            if (std::shared_ptr<PrimitiveObject> spPrim = std::dynamic_pointer_cast<PrimitiveObject>(input_object)) {
                throw makeError<UnimplError>("Unsupport Legacy Primitive Object for creating attribute");
            }
            else if (std::shared_ptr<GeometryObject> spGeo = std::dynamic_pointer_cast<GeometryObject>(input_object)) {
                if (m_attrgroup == "Point") {
                    spGeo->create_attr(ATTR_POINT, attr_name, attr_value);
                }
                else if (m_attrgroup == "Face") {
                    spGeo->create_attr(ATTR_FACE, attr_name, attr_value);
                }
                else if (m_attrgroup == "Geometry") {
                    spGeo->create_attr(ATTR_GEO, attr_name, attr_value);
                }
            }
            else {
                throw makeError<UnimplError>("Unsupport Object for creating attribute");
            }
            return input_object;
        }


        ZPROPERTY(Role = zeno::Role_InputPrimitive, DisplayName = "Attribute Type", Control = zeno::Combobox, ComboBoxItems = ("Vector2", "Vector3", "Vector4", "Integer", "Float", "String", "Boolean"))
        std::string m_type = "Vector3";

        ZPROPERTY(Role = zeno::Role_InputPrimitive, DisplayName = "Attribute Group", Control = zeno::Combobox, ComboBoxItems = ("Point", "Face", "Geometry"))
        std::string m_attrgroup = "Point";

        ZPROPERTY(Role = zeno::Role_InputPrimitive, DisplayName = "Integer Value", Control = zeno::Lineedit, Constrain = "visible = parameter('Attribute Type').value == 'Integer';")
        int m_intValue = 0;

        ZPROPERTY(Role = zeno::Role_InputPrimitive, DisplayName = "Float Value", Control = zeno::Lineedit, Constrain = "visible = parameter('Attribute Type').value == 'Float';")
        float m_floatValue = 0.f;

        ZPROPERTY(Role = zeno::Role_InputPrimitive, DisplayName = "String Value", Control = zeno::Lineedit, Constrain = "visible = parameter('Attribute Type').value == 'String';")
        std::string m_stringValue;

        ZPROPERTY(Role = zeno::Role_InputPrimitive, DisplayName = "Boolean Value", Control = zeno::Checkbox, Constrain = "visible = parameter('Attribute Type').value == 'Boolean';")
        bool m_boolValue = false;

        ZPROPERTY(Role = zeno::Role_InputPrimitive, DisplayName = "Vector2 Value", Control = zeno::Vec2edit, Constrain = "visible = parameter('Attribute Type').value == 'Vector2';")
        zeno::vec2f m_vec2Value;

        ZPROPERTY(Role = zeno::Role_InputPrimitive, DisplayName = "Vector3 Value", Control = zeno::Vec3edit, Constrain = "visible = parameter('Attribute Type').value == 'Vector3';")
        zeno::vec3f m_vec3Value;

        ZPROPERTY(Role = zeno::Role_InputPrimitive, DisplayName = "Vector4 Value", Control = zeno::Vec4edit, Constrain = "visible = parameter('Attribute Type').value == 'Vector4';")
        zeno::vec4f m_vec4Value;
    };

    struct ZDEFNODE() DeleteAttribute : INode {

        ReflectCustomUI m_uilayout = {
            _Group {
                {"input_object", ParamObject("Input", Socket_Clone)},
            },
            _Group {
                {"", ParamObject("Output")},
            }
        };

        zany apply(zany input_object, std::string attr_name = "attribute1") {
            if (attr_name == "") {
                throw makeError<UnimplError>("the attribute name cannot be empty.");
            }

            if (std::shared_ptr<PrimitiveObject> spPrim = std::dynamic_pointer_cast<PrimitiveObject>(input_object)) {
                throw makeError<UnimplError>("Unsupport Legacy Primitive Object for creating attribute");
            }
            else if (std::shared_ptr<GeometryObject> spGeo = std::dynamic_pointer_cast<GeometryObject>(input_object)) {
                if (m_attrgroup == "Point") {
                    spGeo->delete_attr(ATTR_POINT, attr_name);
                }
                else if (m_attrgroup == "Face") {
                    spGeo->delete_attr(ATTR_FACE, attr_name);
                }
                else if (m_attrgroup == "Geometry") {
                    spGeo->delete_attr(ATTR_GEO, attr_name);
                }
            }
            else {
                throw makeError<UnimplError>("Unsupport Object for creating attribute");
            }
            return input_object;
        }


        ZPROPERTY(Role = zeno::Role_InputPrimitive, DisplayName = "Attribute Group", Control = zeno::Combobox, ComboBoxItems = ("Point", "Face", "Geometry"))
        std::string m_attrgroup = "Point";
    };

    struct ZDEFNODE() SetAttribute : INode {

        ReflectCustomUI m_uilayout = {
            _Group {
                {"input_object", ParamObject("Input", Socket_Clone)},
            },
            _Group {
                {"", ParamObject("Output")},
            }
        };

        zany apply(zany input_object, std::string attr_name = "attribute1") {

            if (attr_name == "") {
                throw makeError<UnimplError>("the attribute name cannot be empty.");
            }

            AttrVar attr_value;
            if (m_type == "Vector2") {
                attr_value = m_vec2Value;
            }
            else if (m_type == "Vector3") {
                attr_value = m_vec3Value;
            }
            else if (m_type == "Vector4") {
                attr_value = m_vec4Value;
            }
            else if (m_type == "Integer") {
                attr_value = m_intValue;
            }
            else if (m_type == "Float") {
                attr_value = m_floatValue;
            }
            else if (m_type == "String") {
                attr_value = m_stringValue;
            }
            else if (m_type == "Boolean") {
                attr_value = m_boolValue;
            }

            if (std::shared_ptr<PrimitiveObject> spPrim = std::dynamic_pointer_cast<PrimitiveObject>(input_object)) {
                throw makeError<UnimplError>("Unsupport Legacy Primitive Object for creating attribute");
            }
            else if (std::shared_ptr<GeometryObject> spGeo = std::dynamic_pointer_cast<GeometryObject>(input_object)) {
                if (m_attrgroup == "Point") {
                    if (!spGeo->has_point_attr(attr_name)) {
                        throw makeError<UnimplError>("Input object does not have point attribute: " + attr_name);
                    }
                    spGeo->set_point_attr(attr_name, attr_value);
                }
                else if (m_attrgroup == "Face") {
                    if (!spGeo->has_face_attr(attr_name)) {
                        throw makeError<UnimplError>("Input object does not have face attribute: " + attr_name);
                    }
                    spGeo->set_face_attr(attr_name, attr_value);
                }
                else if (m_attrgroup == "Geometry") {
                    if (!spGeo->has_geometry_attr(attr_name)) {
                        throw makeError<UnimplError>("Input object does not have geometry attribute: " + attr_name);
                    }
                    spGeo->set_geometry_attr(attr_name ,attr_value);
                }
            }
            else {
                throw makeError<UnimplError>("Unsupport Object for creating attribute");
            }
            return input_object;
        }


        ZPROPERTY(Role = zeno::Role_InputPrimitive, DisplayName = "Attribute Type", Control = zeno::Combobox, ComboBoxItems = ("Vector2", "Vector3", "Vector4", "Integer", "Float", "String", "Boolean"))
            std::string m_type = "Vector3";

        ZPROPERTY(Role = zeno::Role_InputPrimitive, DisplayName = "Attribute Group", Control = zeno::Combobox, ComboBoxItems = ("Point", "Face", "Geometry"))
            std::string m_attrgroup = "Point";

        ZPROPERTY(Role = zeno::Role_InputPrimitive, DisplayName = "Integer Value", Control = zeno::Lineedit, Constrain = "visible = parameter('Attribute Type').value == 'Integer';")
            int m_intValue = 0;

        ZPROPERTY(Role = zeno::Role_InputPrimitive, DisplayName = "Float Value", Control = zeno::Lineedit, Constrain = "visible = parameter('Attribute Type').value == 'Float';")
            float m_floatValue = 0.f;

        ZPROPERTY(Role = zeno::Role_InputPrimitive, DisplayName = "String Value", Control = zeno::Lineedit, Constrain = "visible = parameter('Attribute Type').value == 'String';")
            std::string m_stringValue;

        ZPROPERTY(Role = zeno::Role_InputPrimitive, DisplayName = "Boolean Value", Control = zeno::Checkbox, Constrain = "visible = parameter('Attribute Type').value == 'Boolean';")
            bool m_boolValue = false;

        ZPROPERTY(Role = zeno::Role_InputPrimitive, DisplayName = "Vector2 Value", Control = zeno::Vec2edit, Constrain = "visible = parameter('Attribute Type').value == 'Vector2';")
            zeno::vec2f m_vec2Value;

        ZPROPERTY(Role = zeno::Role_InputPrimitive, DisplayName = "Vector3 Value", Control = zeno::Vec3edit, Constrain = "visible = parameter('Attribute Type').value == 'Vector3';")
            zeno::vec3f m_vec3Value;

        ZPROPERTY(Role = zeno::Role_InputPrimitive, DisplayName = "Vector4 Value", Control = zeno::Vec4edit, Constrain = "visible = parameter('Attribute Type').value == 'Vector4';")
            zeno::vec4f m_vec4Value;
    };
}


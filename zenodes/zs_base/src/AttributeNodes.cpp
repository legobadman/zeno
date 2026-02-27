#include "simple_geometry_common.h"

#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace zeno {

namespace {

static std::string get_input2_string(INodeData* nd, const char* name) {
    char buf[512] = {};
    nd->get_input2_string(name, buf, sizeof(buf));
    return std::string(buf);
}

static GeoAttrGroup parse_group(const std::string& s) {
    if (s == "Point") return ATTR_POINT;
    if (s == "Face") return ATTR_FACE;
    if (s == "Geometry") return ATTR_GEO;
    throw std::runtime_error("unknown attribute group");
}

static int group_size(IGeometryObject* geo, GeoAttrGroup grp) {
    if (grp == ATTR_POINT) return geo->npoints();
    if (grp == ATTR_FACE) return geo->nfaces();
    if (grp == ATTR_GEO) return 1;
    return 0;
}

static ZAttrValue make_attr_int(int v) {
    ZAttrValue z{};
    z.type = ATTR_INT;
    z.i = v;
    return z;
}

static ZAttrValue make_attr_float(float v) {
    ZAttrValue z{};
    z.type = ATTR_FLOAT;
    z.f = v;
    return z;
}

static ZAttrValue make_attr_string(const char* s) {
    ZAttrValue z{};
    z.type = ATTR_STRING;
    z.s = s;
    return z;
}

static ZAttrValue make_attr_vec2(Vec2f v) {
    ZAttrValue z{};
    z.type = ATTR_VEC2;
    z.vec2 = v;
    return z;
}

static ZAttrValue make_attr_vec3(Vec3f v) {
    ZAttrValue z{};
    z.type = ATTR_VEC3;
    z.vec3 = v;
    return z;
}

static ZAttrValue make_attr_vec4(Vec4f v) {
    ZAttrValue z{};
    z.type = ATTR_VEC4;
    z.vec4 = v;
    return z;
}

static std::vector<std::string> split_space(const std::string& src) {
    std::istringstream iss(src);
    std::vector<std::string> out;
    std::string token;
    while (iss >> token) out.push_back(token);
    return out;
}

} // namespace

struct CreateAttribute : INode2 {
    NodeType type() const override { return Node_Normal; }
    float time() const override { return 1.0f; }
    void clearCalcResults() override {}
    ZErrorCode apply(INodeData* nd) override {
        auto* geo = nd->clone_input_Geometry("Input");
        const std::string attr_name = get_input2_string(nd, "attr_name");
        const std::string t = get_input2_string(nd, "Attribute Type");
        const std::string g = get_input2_string(nd, "Attribute Group");
        if (attr_name.empty()) throw std::runtime_error("attribute name cannot be empty");
        const GeoAttrGroup grp = parse_group(g);

        if (t == "Vector2") geo->create_attr(grp, attr_name.c_str(), make_attr_vec2(nd->get_input2_vec2f("Vector2 Value")));
        else if (t == "Vector3") geo->create_attr(grp, attr_name.c_str(), make_attr_vec3(nd->get_input2_vec3f("Vector3 Value")));
        else if (t == "Vector4") geo->create_attr(grp, attr_name.c_str(), make_attr_vec4(nd->get_input2_vec4f("Vector4 Value")));
        else if (t == "Integer") geo->create_attr(grp, attr_name.c_str(), make_attr_int(nd->get_input2_int("Integer Value")));
        else if (t == "Float") geo->create_attr(grp, attr_name.c_str(), make_attr_float(nd->get_input2_float("Float Value")));
        else if (t == "String") {
            const std::string s = get_input2_string(nd, "String Value");
            geo->create_attr(grp, attr_name.c_str(), make_attr_string(s.c_str()));
        } else if (t == "Boolean") geo->create_attr(grp, attr_name.c_str(), make_attr_int(nd->get_input2_bool("Boolean Value") ? 1 : 0));
        nd->set_output_object("Output", geo);
        return ZErr_OK;
    }
};

ZENDEFNODE_ABI(CreateAttribute,
    Z_INPUTS(
        {"Input", _gParamType_Geometry},
        {"attr_name", _gParamType_String, ZString("attribute1")},
        {"Attribute Type", _gParamType_String, ZString("Float"), Combobox, Z_STRING_ARRAY("Vector2", "Vector3", "Vector4", "Integer", "Float", "String", "Boolean")},
        {"Attribute Group", _gParamType_String, ZString("Point"), Combobox, Z_STRING_ARRAY("Point", "Face", "Geometry")},
        {"Integer Value", _gParamType_Int, ZInt(0)},
        {"Float Value", _gParamType_Float, ZFloat(0.0f)},
        {"String Value", _gParamType_String, ZString("")},
        {"Boolean Value", _gParamType_Bool, ZInt(0)},
        {"Vector2 Value", _gParamType_Vec2f, ZVec2f(0.0f, 0.0f)},
        {"Vector3 Value", _gParamType_Vec3f, ZVec3f(0.0f, 0.0f, 0.0f)},
        {"Vector4 Value", _gParamType_Vec4f, ZVec4f(0.0f, 0.0f, 0.0f, 0.0f)}
    ),
    Z_OUTPUTS(
        {"Output", _gParamType_Geometry}
    ),
    "geom",
    "",
    "",
    ""
);

struct DeleteAttribute : INode2 {
    NodeType type() const override { return Node_Normal; }
    float time() const override { return 1.0f; }
    void clearCalcResults() override {}
    ZErrorCode apply(INodeData* nd) override {
        auto* geo = nd->clone_input_Geometry("Input");
        const std::string names = get_input2_string(nd, "attr_name");
        const GeoAttrGroup grp = parse_group(get_input2_string(nd, "Attribute Group"));
        if (names.empty()) throw std::runtime_error("attribute name cannot be empty");
        for (const auto& n : split_space(names)) geo->delete_attr(grp, n.c_str());
        nd->set_output_object("Output", geo);
        return ZErr_OK;
    }
};

ZENDEFNODE_ABI(DeleteAttribute,
    Z_INPUTS(
        {"Input", _gParamType_Geometry},
        {"attr_name", _gParamType_String, ZString("attribute1")},
        {"Attribute Group", _gParamType_String, ZString("Point"), Combobox, Z_STRING_ARRAY("Point", "Face", "Geometry")}
    ),
    Z_OUTPUTS(
        {"Output", _gParamType_Geometry}
    ),
    "geom",
    "",
    "",
    ""
);

struct HasAttribute : INode2 {
    NodeType type() const override { return Node_Normal; }
    float time() const override { return 1.0f; }
    void clearCalcResults() override {}
    ZErrorCode apply(INodeData* nd) override {
        auto* geo = nd->get_input_Geometry("Input");
        const std::string name = get_input2_string(nd, "attr_name");
        const GeoAttrGroup grp = parse_group(get_input2_string(nd, "Attribute Group"));
        if (name.empty()) throw std::runtime_error("attribute name cannot be empty");
        nd->set_output_int("hasAttr", geo->has_attr(grp, name.c_str()) ? 1 : 0);
        return ZErr_OK;
    }
};

ZENDEFNODE_ABI(HasAttribute,
    Z_INPUTS(
        {"Input", _gParamType_Geometry},
        {"attr_name", _gParamType_String, ZString("attribute1")},
        {"Attribute Group", _gParamType_String, ZString("Point"), Combobox, Z_STRING_ARRAY("Point", "Face", "Geometry")}
    ),
    Z_OUTPUTS(
        {"hasAttr", _gParamType_Int}
    ),
    "geom",
    "",
    "",
    ""
);

struct CopyAttributeFrom : INode2 {
    NodeType type() const override { return Node_Normal; }
    float time() const override { return 1.0f; }
    void clearCalcResults() override {}
    ZErrorCode apply(INodeData* nd) override {
        auto* input = nd->clone_input_Geometry("Input");
        auto* from = nd->get_input_Geometry("From");
        const std::string source = get_input2_string(nd, "Source Attribute");
        const std::string target = get_input2_string(nd, "Target Attribute");
        const GeoAttrGroup grp = parse_group(get_input2_string(nd, "Attribute Group"));
        const int n = group_size(from, grp);
        if (from->has_attr(grp, source.c_str(), ATTR_FLOAT)) {
            std::vector<float> buf(static_cast<std::size_t>(n), 0.0f);
            from->get_float_attr(grp, source.c_str(), buf.data(), buf.size());
            input->delete_attr(grp, target.c_str());
            input->create_attr_by_float(grp, target.c_str(), buf.data(), buf.size());
        } else if (from->has_attr(grp, source.c_str(), ATTR_VEC3)) {
            std::vector<Vec3f> buf(static_cast<std::size_t>(n));
            from->get_vec3f_attr(grp, source.c_str(), buf.data(), buf.size());
            input->delete_attr(grp, target.c_str());
            input->create_attr_by_vec3(grp, target.c_str(), buf.data(), buf.size());
        } else {
            throw std::runtime_error("CopyAttributeFrom supports float/vec3 only in ABI");
        }
        nd->set_output_object("Output", input);
        return ZErr_OK;
    }
};

ZENDEFNODE_ABI(CopyAttributeFrom,
    Z_INPUTS(
        {"Input", _gParamType_Geometry},
        {"From", _gParamType_Geometry},
        {"Source Attribute", _gParamType_String, ZString("")},
        {"Target Attribute", _gParamType_String, ZString("")},
        {"Attribute Group", _gParamType_String, ZString("Point"), Combobox, Z_STRING_ARRAY("Point", "Face", "Geometry")}
    ),
    Z_OUTPUTS(
        {"Output", _gParamType_Geometry}
    ),
    "geom",
    "",
    "",
    ""
);

struct CopyAttribute : INode2 {
    NodeType type() const override { return Node_Normal; }
    float time() const override { return 1.0f; }
    void clearCalcResults() override {}
    ZErrorCode apply(INodeData* nd) override {
        auto* input = nd->clone_input_Geometry("Input");
        const std::string source = get_input2_string(nd, "Source Attribute");
        const std::string target = get_input2_string(nd, "Target Attribute");
        const GeoAttrGroup grp = parse_group(get_input2_string(nd, "Attribute Group"));
        const int n = group_size(input, grp);
        if (input->has_attr(grp, source.c_str(), ATTR_FLOAT)) {
            std::vector<float> buf(static_cast<std::size_t>(n), 0.0f);
            input->get_float_attr(grp, source.c_str(), buf.data(), buf.size());
            input->delete_attr(grp, target.c_str());
            input->create_attr_by_float(grp, target.c_str(), buf.data(), buf.size());
        } else if (input->has_attr(grp, source.c_str(), ATTR_VEC3)) {
            std::vector<Vec3f> buf(static_cast<std::size_t>(n));
            input->get_vec3f_attr(grp, source.c_str(), buf.data(), buf.size());
            input->delete_attr(grp, target.c_str());
            input->create_attr_by_vec3(grp, target.c_str(), buf.data(), buf.size());
        } else {
            throw std::runtime_error("CopyAttribute supports float/vec3 only in ABI");
        }
        nd->set_output_object("Output", input);
        return ZErr_OK;
    }
};

ZENDEFNODE_ABI(CopyAttribute,
    Z_INPUTS(
        {"Input", _gParamType_Geometry},
        {"Source Attribute", _gParamType_String, ZString("")},
        {"Target Attribute", _gParamType_String, ZString("")},
        {"Attribute Group", _gParamType_String, ZString("Point"), Combobox, Z_STRING_ARRAY("Point", "Face", "Geometry")}
    ),
    Z_OUTPUTS(
        {"Output", _gParamType_Geometry}
    ),
    "geom",
    "",
    "",
    ""
);

struct SetAttribute : INode2 {
    NodeType type() const override { return Node_Normal; }
    float time() const override { return 1.0f; }
    void clearCalcResults() override {}
    ZErrorCode apply(INodeData* nd) override {
        auto* geo = nd->clone_input_Geometry("Input");
        const std::string attr_name = get_input2_string(nd, "attr_name");
        const std::string t = get_input2_string(nd, "Attribute Type");
        const GeoAttrGroup grp = parse_group(get_input2_string(nd, "Attribute Group"));
        if (attr_name.empty()) throw std::runtime_error("attribute name cannot be empty");

        if (t == "Vector2") geo->set_attr2(grp, attr_name.c_str(), make_attr_vec2(nd->get_input2_vec2f("Vector2 Value")));
        else if (t == "Vector3") geo->set_attr2(grp, attr_name.c_str(), make_attr_vec3(nd->get_input2_vec3f("Vector3 Value")));
        else if (t == "Vector4") geo->set_attr2(grp, attr_name.c_str(), make_attr_vec4(nd->get_input2_vec4f("Vector4 Value")));
        else if (t == "Integer") geo->set_attr2(grp, attr_name.c_str(), make_attr_int(nd->get_input2_int("Integer Value")));
        else if (t == "Float") geo->set_attr2(grp, attr_name.c_str(), make_attr_float(nd->get_input2_float("Float Value")));
        else if (t == "String") {
            const std::string s = get_input2_string(nd, "String Value");
            geo->set_attr2(grp, attr_name.c_str(), make_attr_string(s.c_str()));
        } else if (t == "Boolean") geo->set_attr2(grp, attr_name.c_str(), make_attr_int(nd->get_input2_bool("Boolean Value") ? 1 : 0));
        nd->set_output_object("Output", geo);
        return ZErr_OK;
    }
};

ZENDEFNODE_ABI(SetAttribute,
    Z_INPUTS(
        {"Input", _gParamType_Geometry},
        {"attr_name", _gParamType_String, ZString("attribute1")},
        {"Attribute Type", _gParamType_String, ZString("Float"), Combobox, Z_STRING_ARRAY("Vector2", "Vector3", "Vector4", "Integer", "Float", "String", "Boolean")},
        {"Attribute Group", _gParamType_String, ZString("Point"), Combobox, Z_STRING_ARRAY("Point", "Face", "Geometry")},
        {"Integer Value", _gParamType_Int, ZInt(0)},
        {"Float Value", _gParamType_Float, ZFloat(0.0f)},
        {"String Value", _gParamType_String, ZString("")},
        {"Boolean Value", _gParamType_Bool, ZInt(0)},
        {"Vector2 Value", _gParamType_Vec2f, ZVec2f(0.0f, 0.0f)},
        {"Vector3 Value", _gParamType_Vec3f, ZVec3f(0.0f, 0.0f, 0.0f)},
        {"Vector4 Value", _gParamType_Vec4f, ZVec4f(0.0f, 0.0f, 0.0f, 0.0f)}
    ),
    Z_OUTPUTS(
        {"Output", _gParamType_Geometry}
    ),
    "geom",
    "",
    "",
    ""
);

} // namespace zeno


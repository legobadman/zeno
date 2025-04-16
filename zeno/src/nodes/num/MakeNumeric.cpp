#include <zeno/zeno.h>
#include <zeno/types/NumericObject.h>

namespace {

struct NumericInt : zeno::INode {
    virtual void apply() override {
        ZImpl(set_primitive_output("value", ZImpl(get_param<int>("value"))));
    }
};

ZENDEFNODE(NumericInt, {
    {},
    {{gParamType_Int, "value"}},
    {{gParamType_Int, "value", "0"}},
    {"numeric"},
});


struct NumericIntVec2 : zeno::INode {
    virtual void apply() override {
        auto x = ZImpl(get_param<int>("x"));
        auto y = ZImpl(get_param<int>("y"));
        ZImpl(set_primitive_output("vec2", zeno::vec2i(x, y)));
    }
};

ZENDEFNODE(NumericIntVec2, {
    {},
    {{gParamType_Vec2i, "vec2"}},
    {{gParamType_Int, "x", "0"}, {gParamType_Int, "y", "0"}},
    {"deprecated"},
});


struct PackNumericIntVec2 : zeno::INode {
    virtual void apply() override {
        auto x = ZImpl(get_input2<int>("x"));
        auto y = ZImpl(get_input2<int>("y"));
        ZImpl(set_primitive_output("vec2", zeno::vec2i(x, y)));
    }
};

ZENDEFNODE(PackNumericIntVec2, {
    {{gParamType_Int, "x", "0"}, {gParamType_Int, "y", "0"}},
    {{"vec2i", "vec2"}},
    {},
    {"deprecated"},
});


struct NumericIntVec3 : zeno::INode {
    virtual void apply() override {
        auto x = ZImpl(get_param<int>("x"));
        auto y = ZImpl(get_param<int>("y"));
        auto z = ZImpl(get_param<int>("z"));
        ZImpl(set_primitive_output("vec3", zeno::vec3i(x, y, z)));
    }
};

ZENDEFNODE(NumericIntVec3, {
    {},
    {{gParamType_Vec3i, "vec3"}},
    {{gParamType_Int, "x", "0"}, {gParamType_Int, "y", "0"}, {gParamType_Int, "z", "0"}},
    {"numeric"},
});


struct NumericIntVec4 : zeno::INode {
    virtual void apply() override {
        auto x = ZImpl(get_param<int>("x"));
        auto y = ZImpl(get_param<int>("y"));
        auto z = ZImpl(get_param<int>("z"));
        auto w = ZImpl(get_param<int>("w"));
        ZImpl(set_primitive_output("vec4", zeno::vec4i(x, y, z, w)));
    }
};

ZENDEFNODE(NumericIntVec4, {
    {},
    {{gParamType_Vec4i, "vec4"}},
    {{gParamType_Float, "x", "0"}, {gParamType_Float, "y", "0"},
     {gParamType_Float, "z", "0"}, {gParamType_Float, "w", "0"}},
    {"numeric"},
});


struct NumericFloat : zeno::INode {
    virtual void apply() override {
        ZImpl(set_primitive_output("value", ZImpl(get_param<float>("value"))));
    }
};

ZENDEFNODE(NumericFloat, {
    {},
    {{gParamType_Float, "value"}},
    {{gParamType_Float, "value", "0"}},
    {"numeric"},
});


struct NumericVec2 : zeno::INode {
    virtual void apply() override {
        auto x = ZImpl(get_param<float>("x"));
        auto y = ZImpl(get_param<float>("y"));
        ZImpl(set_primitive_output("vec2", zeno::vec2f(x, y)));
    }
};

ZENDEFNODE(NumericVec2, {
    {},
    {{gParamType_Vec2f, "vec2"}},
    {{gParamType_Float, "x", "0"}, {gParamType_Float, "y", "0"}},
    {"numeric"},
});


struct NumericVec3 : zeno::INode {
    virtual void apply() override {
        auto x = ZImpl(get_param<float>("x"));
        auto y = ZImpl(get_param<float>("y"));
        auto z = ZImpl(get_param<float>("z"));
        ZImpl(set_primitive_output("vec3", zeno::vec3f(x, y, z)));
    }
};

ZENDEFNODE(NumericVec3, {
    {},
    {{gParamType_Vec3f, "vec3"}},
    {{gParamType_Float, "x", "0"}, {gParamType_Float, "y", "0"}, {gParamType_Float, "z", "0"}},
    {"numeric"},
});


struct NumericVec4 : zeno::INode {
    virtual void apply() override {
        auto x = ZImpl(get_param<float>("x"));
        auto y = ZImpl(get_param<float>("y"));
        auto z = ZImpl(get_param<float>("z"));
        auto w = ZImpl(get_param<float>("w"));
        ZImpl(set_primitive_output("vec2", zeno::vec4f(x, y, z, w)));
    }
};

ZENDEFNODE(NumericVec4, {
    {},
    {{gParamType_Vec4f, "vec4"}},
    {{gParamType_Float, "x", "0"}, {gParamType_Float, "y", "0"},
     {gParamType_Float, "z", "0"}, {gParamType_Float, "w", "0"}},
    {"numeric"},
});

struct PackNumericVecInt : zeno::INode {
    virtual void apply() override {
        auto _type = ZImpl(get_param<std::string>("type"));
        auto x = ZImpl(get_input2<int>("x"));
        auto y = ZImpl(get_input2<int>("y"));
        auto z = ZImpl(get_input2<int>("z"));
        auto w = ZImpl(get_input2<int>("w"));
        if (_type == "int") {
            ZImpl(set_primitive_output("veci", x));
        } else if (_type == "vec2i") {
            ZImpl(set_primitive_output("veci", zeno::vec2i(x, y)));
        } else if (_type == "vec3i") {
            ZImpl(set_primitive_output("veci", zeno::vec3i(x, y, z)));
        } else if (_type == "vec4i") {
            ZImpl(set_primitive_output("veci", zeno::vec4f(x, y, z, w)));
        }
    }
};

ZENDEFNODE(PackNumericVecInt, {
    {
        {gParamType_Int, "x", "0"},
        {gParamType_Int, "y", "0"},
        {gParamType_Int, "z", "0"},
        {gParamType_Int, "w", "0"},
    },
    {{"NumericObject","veci"}},
    {
        {"enum int vec2i vec3i vec4i", "type", "vec3i"},
    },
    {"numeric"},
});

struct PackNumericVec : zeno::INode {
    virtual void apply() override {
        auto _type = ZImpl(get_param<std::string>("type"));
        auto x = ZImpl(get_input2<float>("x"));
        auto y = ZImpl(get_input2<float>("y"));
        auto z = ZImpl(get_input2<float>("z"));
        auto w = ZImpl(get_input2<float>("w"));
        if (_type == "float") {
            ZImpl(set_primitive_output("vec", x));
        } else if (_type == "vec2f") {
            ZImpl(set_primitive_output("vec", zeno::vec2f(x, y)));
        } else if (_type == "vec3f") {
            ZImpl(set_primitive_output("vec", zeno::vec3f(x, y, z)));
        } else if (_type == "vec4f") {
            ZImpl(set_primitive_output("vec", zeno::vec4f(x, y, z, w)));
        }
    }
};

ZENDEFNODE(PackNumericVec, {
    {
        {gParamType_Float, "x", "0"},
        {gParamType_Float, "y", "0"},
        {gParamType_Float, "z", "0"},
        {gParamType_Float, "w", "0"},
    },
    {{"NumericObject","vec"}},
    {
        {"enum float vec2f vec3f vec4f", "type", "vec3f"},
    },
    {"numeric"},
});

}

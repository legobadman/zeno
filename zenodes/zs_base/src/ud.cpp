#include "simple_geometry_common.h"

#include <string>

namespace zeno {

namespace {

static std::string get_input2_string(INodeData* nd, const char* name) {
    char buf[512] = {};
    nd->get_input2_string(name, buf, sizeof(buf));
    return std::string(buf);
}

static Vec3f make_vec3(float x, float y, float z) {
    return Vec3f{x, y, z};
}

} // namespace

struct GetUserData3 : INode2 {
    NodeType type() const override { return Node_Normal; }
    float time() const override { return 1.0f; }
    void clearCalcResults() override {}

    ZErrorCode apply(INodeData* nd) override {
        auto* obj = nd->get_input_object("object");
        if (!obj) {
            nd->set_output_bool("hasValue", false);
            nd->set_output_vec3f("data", make_vec3(0.0f, 0.0f, 0.0f));
            return ZErr_OK;
        }

        auto* ud = obj->userData();
        if (!ud) {
            nd->set_output_bool("hasValue", false);
            nd->set_output_vec3f("data", make_vec3(0.0f, 0.0f, 0.0f));
            return ZErr_OK;
        }

        const std::string key = get_input2_string(nd, "key");
        bool has_value = false;
        Vec3f out = make_vec3(0.0f, 0.0f, 0.0f);

        if (ud->has_vec3f(key.c_str())) {
            out = ud->get_vec3f(key.c_str(), make_vec3(0.0f, 0.0f, 0.0f));
            has_value = true;
        } else if (ud->has_vec4f(key.c_str())) {
            const Vec4f v = ud->get_vec4f(key.c_str());
            out = make_vec3(v.x, v.y, v.z);
            has_value = true;
        } else if (ud->has_vec2f(key.c_str())) {
            const Vec2f v = ud->get_vec2f(key.c_str(), Vec2f{0.0f, 0.0f});
            out = make_vec3(v.x, v.y, 0.0f);
            has_value = true;
        } else if (ud->has_float(key.c_str())) {
            const float v = ud->get_float(key.c_str(), 0.0f);
            out = make_vec3(v, 0.0f, 0.0f);
            has_value = true;
        } else if (ud->has_int(key.c_str())) {
            const int v = ud->get_int(key.c_str(), 0);
            out = make_vec3(static_cast<float>(v), 0.0f, 0.0f);
            has_value = true;
        } else if (ud->has(key.c_str())) {
            has_value = true;
        }

        nd->set_output_bool("hasValue", has_value);
        nd->set_output_vec3f("data", out);
        return ZErr_OK;
    }
};

ZENDEFNODE_ABI(GetUserData3,
    Z_INPUTS(
        {"object", _gParamType_IObject},
        {"key", _gParamType_String, ZString("pos")}
    ),
    Z_OUTPUTS(
        {"data", _gParamType_Vec3f},
        {"hasValue", _gParamType_Bool}
    ),
    "lifecycle",
    "",
    "",
    ""
);

} // namespace zeno

